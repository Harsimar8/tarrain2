import * as Cesium from "cesium";
import fs from "fs";
import path from "path";

// --------------------------------------------------
// Cesium ion access token
// --------------------------------------------------
Cesium.Ion.defaultAccessToken =
  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiIyNDZmY2UwNi0yZDc0LTRlMDYtODI5NC04MzBmMWM3NzdlNDciLCJpZCI6NDQyMjYxLCJzdWIiOiJIYXJzaW1hcjA4IiwiaXNzIjoiaHR0cHM6Ly9hcGkuY2VzaXVtLmNvbSIsImF1ZCI6IlVudGl0bGVkIiwiaWF0IjoxNzgyNzY2NDk4fQ.OxFO8YZ1QuoUOdpW0R-yAtg9K-UFsZA8MwfvFeYxLOI";

// --------------------------------------------------
// Input and output folders
// --------------------------------------------------
const tilesDir = "../output/tiles";

// Output folder
const elevationsDir = "../output/elevations";

// Create output folder if it does not exist
if (!fs.existsSync(elevationsDir)) {
  fs.mkdirSync(elevationsDir, { recursive: true });
}

// --------------------------------------------------
// Process one tile
// --------------------------------------------------
async function processTile(terrain, filename) {
  const inputPath = path.join(tilesDir, filename);

  const data = JSON.parse(
    fs.readFileSync(inputPath, "utf8")
  );

  const positions = [];
  const buildingRanges = [];

  // ------------------------------------------------
  // First position = tile center
  // ------------------------------------------------
  positions.push(
    Cesium.Cartographic.fromDegrees(
      data.tileCenter.lon,
      data.tileCenter.lat
    )
  );

  // ------------------------------------------------
  // Add every footprint vertex of every building
  // ------------------------------------------------
  for (const building of data.buildings) {
    const start = positions.length;

    for (const node of building.nodes) {
      positions.push(
        Cesium.Cartographic.fromDegrees(
          node.lon,
          node.lat
        )
      );
    }

    const end = positions.length;

    buildingRanges.push({
      id: building.id,
      start,
      end
    });
  }

  // ------------------------------------------------
  // Sample terrain
  // ------------------------------------------------
  const updated =
    await Cesium.sampleTerrainMostDetailed(
      terrain,
      positions
    );

  const result = {
    tileCenterElevation: updated[0].height,
    buildings: {}
  };

  // ------------------------------------------------
  // Average elevation of each building footprint
  // ------------------------------------------------
  for (const r of buildingRanges) {
    let sum = 0.0;
    let count = 0;

    for (let i = r.start; i < r.end; i++) {
      sum += updated[i].height;
      count++;
    }

    result.buildings[r.id] =
      count > 0 ? sum / count : 0.0;
  }

  const outputFile = filename.replace(
    "_terrain_input.json",
    "_elevations.json"
  );

  fs.writeFileSync(
    path.join(elevationsDir, outputFile),
    JSON.stringify(result, null, 2)
  );
}

// --------------------------------------------------
// Process all tiles
// --------------------------------------------------
async function sampleTerrain() {
  const terrain =
    await Cesium.createWorldTerrainAsync();

  const files = fs
    .readdirSync(tilesDir)
    .filter(f =>
      f.endsWith("_terrain_input.json")
    );

  console.log(
    `Found ${files.length} tile files`
  );

  let processed = 0;

  for (const file of files) {
    await processTile(terrain, file);

    processed++;

    if (processed % 20 === 0) {
      console.log(
        `${processed}/${files.length} tiles processed`
      );
    }
  }

  console.log(
    `Done. Processed ${processed} tiles.`
  );
}

sampleTerrain().catch(console.error);