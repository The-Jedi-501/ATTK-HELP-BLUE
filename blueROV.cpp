/*
 * ============================================================
 *  blueROV.cpp
 *  BlueROV2 Attacker Agent Simulation
 *  Author: Nadeem
 * ============================================================
 *
 *  OVERVIEW:
 *  ---------
 *  Simulates a BlueROV2 UUV retrofitted for a one-way harbor
 *  attack mission. Loads a real map of Pearl Harbor from a
 *  shapefile, converts it to a navigable grid, and runs the
 *  attacker agent from an ocean entry point toward the harbor.
 *
 *  REAL-WORLD BLUEROV2 SPECS USED IN THIS SIMULATION:
 *  ---------------------------------------------------
 *    Unit cost              : ~$6,000 base (unmodified)
 *    Speed                  : 1–3 knots
 *    Shallow water capable  : Yes
 *    Emission frequency     : 300k–450k Hz (integration dependent)
 *    Mission type           : One-way (expendable)
 *
 *  WHAT YOU SEE ON SCREEN:
 *  -----------------------
 *    Dark blue cells  = navigable water
 *    Dark green cells = land / obstacles
 *    Red circle       = BlueROV2 attacker agent
 *    Orange marker    = ocean entry point (where attacker spawns)
 *    White marker     = harbor target (where attacker is heading)
 *
 *  HOW TO BUILD AND RUN:
 *  ----------------------
 *    cmake --build build
 *    ./build/blueROV
 *
 *  WHAT HAPPENS:
 *  -------------
 *    1. Map loads and window opens
 *    2. Entry point (orange) and target (white) are marked
 *    3. Red circle spawns at entry point and moves toward target
 *    4. Terminal prints progress updates along the way
 *    5. When attacker reaches target, mission complete is printed
 * ============================================================
 */

#include "mapCreation.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

int main() {

    // load pearl harbor map — 0 = water, 1 = land
    MapCreation harbor("Maps/pearlHarbour/Harbour_Depth_Area.shp", 100);    //Checks to see if the map loads properly
    if (harbor.getCellsN() == 0) {
        std::cerr << "CRITICAL: Map failed to load. Ensure maps/ folder contains .shp, .shx, and .dbf files." << std::endl;
        return 1;
}
    int n = harbor.getCellsN();
    const auto& grid = harbor.getGrid();

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(700, 740)), "BlueROV2 Attacker Sim");
    window.setFramerateLimit(30);

    float cellSize = 700.0f / n;

    //NADEEM
    // // BlueROV2 specs: ~1-3 knots, shallow water capable, one-way mission
    // float posRow = 5.0f,  posCol = 5.0f;   // ocean entry point
    // float tgtRow = 80.0f, tgtCol = 50.0f;  // harbor target
    // float speed  = 0.02f;                  // 1-3 knot equivalent
    // bool  active = true;

//CHRIS
    float posRow = 8.0f,  posCol = 15.0f;   // ocean entry point
    float tgtRow = 45.0f, tgtCol = 35.0f;   // harbor target
    float speed  = 0.003f;                  // slower so you can see it

    std::cout << "BlueROV2 launched\n";

    sf::RectangleShape tile(sf::Vector2f(cellSize, cellSize));

    while (window.isOpen()) {

        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>())
                window.close();
        }

        // move attacker toward harbor target
        if (active) {
            float dr   = tgtRow - posRow;
            float dc   = tgtCol - posCol;
            float dist = std::sqrt(dr * dr + dc * dc);

            if (dist < speed) {
                active = false;
                std::cout << "BlueROV2 reached harbor target\n";
            } else {
                posRow += (dr / dist) * speed;
                posCol += (dc / dist) * speed;
            }
        }
//chris fix
        if (active && (int)(posRow) % 10 == 0) {
        std::cout << "BlueROV2 position: " << posRow << ", " << posCol << "\n";
        }

        window.clear(sf::Color::Black);

        // draw map
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) {
                tile.setPosition(sf::Vector2f(c * cellSize, r * cellSize));
                tile.setFillColor(grid[r][c] == 0 ? sf::Color(20, 50, 120)
                                                   : sf::Color(50, 110, 50));
                window.draw(tile);
            }
        }

        // draw harbor target (white marker)
        {
            float s = cellSize * 0.45f;
            sf::RectangleShape marker(sf::Vector2f(s, s));
            marker.setOrigin(sf::Vector2f(s / 2.f, s / 2.f));
            marker.setPosition({ (tgtCol + 0.5f) * cellSize, (tgtRow + 0.5f) * cellSize });
            marker.setRotation(sf::degrees(45.f));
            marker.setFillColor(sf::Color::White);
            window.draw(marker);
        }

        // draw BlueROV2 (red circle)
        if (active) {
            float r = cellSize * 0.45f;
            sf::CircleShape agent(r);
            agent.setOrigin(sf::Vector2f(r, r));
            agent.setPosition({ (posCol + 0.5f) * cellSize, (posRow + 0.5f) * cellSize });
            agent.setFillColor(sf::Color(255, 60, 60));
            agent.setOutlineColor(sf::Color::White);
            agent.setOutlineThickness(1.5f);
            window.draw(agent);
        }

        window.display();
    }

    return 0;
}