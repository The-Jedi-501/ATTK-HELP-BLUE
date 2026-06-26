/*
    Sacrificial Decoy Demo
    
    Purpose: Show how a decoy agent can intercept a seeker's pathfinding
    and redirect it away from the real target
    
    How it works:
    1) Click to place the TARGET (what the seeker wants to reach)
    2) Click to place the DECOY (the fake target)
    3) Click to place the SEEKER (the attacker)
    4) Watch the seeker get pulled toward the decoy instead of the target
    5) When seeker reaches decoy, decoy is destroyed and seeker resumes toward real target
    
    To build and run:
    cd build
    make
    cd ..
    ./build/DecoyDemo
    
    No new headers needed beyond what patrol demo used
*/

#include "mapCreation.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <string>

// helper function — calculates distance between two points on the grid
// same math as patrol demo: sqrt(dr^2 + dc^2)
float distance(float r1, float c1, float r2, float c2) {
    float dr = r2 - r1;
    float dc = c2 - c1;
    return std::sqrt(dr * dr + dc * dc);
}

int main() {
    std::cout << "DECOY DEMO STARTED\n";
    
    // load Pearl Harbor map — same as patrol demo
    MapCreation map("Maps/pearlHarbour/Harbour_Depth_Area.shp", 100);
    
    // open window
    sf::RenderWindow window(
        sf::VideoMode(sf::Vector2u(700, 740)),
        "Sacrificial Decoy Demo"
    );
    window.setFramerateLimit(30);
    
    // grid info — same as patrol demo
    int n = map.getCellsN();
    float cellSize = 700.0f / n;
    const auto& grid = map.getGrid();
    
    // ── AGENT POSITIONS ──────────────────────────────────────────────
    // all start at -1 meaning not placed yet
    
    int targetRow = -1, targetCol = -1;   // where the seeker wants to go
    int decoyRow  = -1, decoyCol  = -1;   // the fake target
    float seekerRow = -1, seekerCol = -1; // the attacker (float so it can move smoothly)
    
    // ── STATE MACHINE ────────────────────────────────────────────────
    // tracks what we're waiting for next
    // "place_target" -> "place_decoy" -> "place_seeker" -> "running" -> "done"
    std::string state = "place_target";
    
    // ── DECOY STATE ──────────────────────────────────────────────────
    bool decoyAlive = true;   // decoy starts alive, gets destroyed when seeker reaches it
    
    // ── SEEKER MOVEMENT ──────────────────────────────────────────────
    // seeker always moves toward a "current target" position
    // this gets switched from real target to decoy when attraction triggers
    float seekerTargetRow = -1, seekerTargetCol = -1;
    
    float speed = 0.15f;              // how fast seeker moves each frame
    float attractionRadius = 12.0f;   // how close seeker needs to be to detect decoy (in grid cells)
    float reachRadius = 1.0f;         // how close = "arrived"
    
    // ── TRACKING WHETHER SEEKER IS CURRENTLY CHASING DECOY ──────────
    bool chasingDecoy = false;
    
    std::cout << "Click a water cell to place the TARGET (blue square)\n";
    
    while (window.isOpen()) {
        
        // ── EVENT HANDLING ───────────────────────────────────────────
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            
            // handle mouse clicks
            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (click->button == sf::Mouse::Button::Left) {
                    
                    // convert pixel click to grid row/col — same as patrol demo
                    int col = static_cast<int>(click->position.x / cellSize);
                    int row = static_cast<int>(click->position.y / cellSize);
                    
                    // bounds check — make sure click is inside the grid
                    if (row < 0 || row >= n || col < 0 || col >= n) continue;
                    
                    // only allow clicks on water cells
                    if (grid[row][col] != 0) {
                        std::cout << "That's land — pick a water cell\n";
                        continue;
                    }
                    
                    // place agents one at a time based on current state
                    if (state == "place_target") {
                        targetRow = row;
                        targetCol = col;
                        state = "place_decoy";
                        std::cout << "Target placed! Now click to place the DECOY (yellow circle)\n";
                    }
                    else if (state == "place_decoy") {
                        decoyRow = row;
                        decoyCol = col;
                        state = "place_seeker";
                        std::cout << "Decoy placed! Now click to place the SEEKER (red triangle)\n";
                    }
                    else if (state == "place_seeker") {
                        seekerRow = row;
                        seekerCol = col;
                        // seeker starts by heading toward the real target
                        seekerTargetRow = targetRow;
                        seekerTargetCol = targetCol;
                        state = "running";
                        std::cout << "Seeker placed! Watch it get redirected by the decoy\n";
                    }
                }
            }
        }
        
        // ── SIMULATION UPDATE ─────────────────────────────────────────
        if (state == "running") {
            
            // ── ATTRACTION CHECK ─────────────────────────────────────
            // if decoy is still alive, check if seeker is close enough to detect it
            if (decoyAlive && !chasingDecoy) {
                float distToDecoy = distance(seekerRow, seekerCol, decoyRow, decoyCol);
                
                if (distToDecoy < attractionRadius) {
                    // seeker detected the decoy — redirect it
                    seekerTargetRow = decoyRow;
                    seekerTargetCol = decoyCol;
                    chasingDecoy = true;
                    std::cout << "Decoy detected! Seeker redirected away from real target\n";
                }
            }
            
            // ── CHECK IF SEEKER REACHED THE DECOY ────────────────────
            if (decoyAlive && chasingDecoy) {
                float distToDecoy = distance(seekerRow, seekerCol, decoyRow, decoyCol);
                
                if (distToDecoy < reachRadius) {
                    // seeker reached the decoy — decoy is destroyed
                    decoyAlive = false;
                    chasingDecoy = false;
                    // seeker now resumes heading toward the real target
                    seekerTargetRow = targetRow;
                    seekerTargetCol = targetCol;
                    std::cout << "Decoy destroyed! Seeker resuming toward real target\n";
                }
            }
            
            // ── CHECK IF SEEKER REACHED THE REAL TARGET ──────────────
            float distToTarget = distance(seekerRow, seekerCol, targetRow, targetCol);
            if (distToTarget < reachRadius) {
                state = "done";
                std::cout << "Seeker reached the target!\n";
            }
            
            // ── MOVE SEEKER ───────────────────────────────────────────
            // same movement math as patrol demo
            // figure out direction toward current target then move one step
            float dr = seekerTargetRow - seekerRow;
            float dc = seekerTargetCol - seekerCol;
            float dist = std::sqrt(dr * dr + dc * dc);
            
            if (dist > speed) {
                // normalize direction then multiply by speed
                seekerRow += (dr / dist) * speed;
                seekerCol += (dc / dist) * speed;
            }
        }
        
        // ── DRAWING ───────────────────────────────────────────────────
        window.clear(sf::Color::Black);
        
        // draw map grid — exactly the same as patrol demo
        sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));
        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                cell.setPosition(sf::Vector2f(col * cellSize, row * cellSize));
                if (grid[row][col] == 0)
                    cell.setFillColor(sf::Color(20, 50, 120));   // water = blue
                else
                    cell.setFillColor(sf::Color(50, 110, 50));   // land = green
                window.draw(cell);
            }
        }
        
        // draw TARGET — blue square
        if (targetRow != -1) {
            float cx = (targetCol + 0.5f) * cellSize;
            float cy = (targetRow + 0.5f) * cellSize;
            float s = cellSize * 0.6f;
            sf::RectangleShape t(sf::Vector2f(s, s));
            t.setOrigin(sf::Vector2f(s / 2.f, s / 2.f));
            t.setPosition(sf::Vector2f(cx, cy));
            t.setFillColor(sf::Color(30, 100, 220));   // blue
            t.setOutlineColor(sf::Color::White);
            t.setOutlineThickness(1.5f);
            window.draw(t);
        }
        
        // draw DECOY — yellow circle with attraction radius ring
        // only draw if decoy is still alive
        if (decoyAlive && decoyRow != -1) {
            float cx = (decoyCol + 0.5f) * cellSize;
            float cy = (decoyRow + 0.5f) * cellSize;
            
            // draw attraction radius as faint yellow ring
            // this shows the player how close the seeker needs to get
            float radiusPixels = attractionRadius * cellSize;
            sf::CircleShape ring(radiusPixels);
            ring.setOrigin(sf::Vector2f(radiusPixels, radiusPixels));
            ring.setPosition(sf::Vector2f(cx, cy));
            ring.setFillColor(sf::Color(255, 200, 0, 20));     // very faint yellow fill
            ring.setOutlineColor(sf::Color(255, 200, 0, 80));  // semi transparent yellow border
            ring.setOutlineThickness(1.0f);
            ring.setPointCount(48);
            window.draw(ring);
            
            // draw the decoy itself on top
            sf::CircleShape d(cellSize * 0.4f);
            d.setOrigin(sf::Vector2f(cellSize * 0.4f, cellSize * 0.4f));
            d.setPosition(sf::Vector2f(cx, cy));
            d.setFillColor(sf::Color(255, 200, 0));   // yellow
            d.setOutlineColor(sf::Color::White);
            d.setOutlineThickness(1.5f);
            window.draw(d);
        }
        
        // draw SEEKER — red triangle
        if (seekerRow != -1) {
            float cx = (seekerCol + 0.5f) * cellSize;
            float cy = (seekerRow + 0.5f) * cellSize;
            float r = cellSize * 0.45f;
            sf::CircleShape s(r, 3);   // 3 points = triangle
            s.setOrigin(sf::Vector2f(r, r));
            s.setPosition(sf::Vector2f(cx, cy));
            s.setFillColor(sf::Color(220, 30, 30));   // red
            s.setOutlineColor(sf::Color::White);
            s.setOutlineThickness(1.5f);
            window.draw(s);
        }
        
        window.display();
    }
    
    return 0;
}