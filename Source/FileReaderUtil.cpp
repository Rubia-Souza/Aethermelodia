//
// Created by Aline on 20/06/2025.
//

#include "FileReaderUtil.h"
#include "SDL_log.h"

std::vector<Note> FileReaderUtil::loadChartManually(const std::string& filePath) {
    std::ifstream chartFile(filePath);
    std::string line;

    if (!chartFile.is_open())
    {
        SDL_Log("Failed to load paths");
    }

    long resolution = 192; // Valor padrão
    std::vector<BpmChangeEvent> bpmEvents;
    std::vector<std::pair<int, int>> rawNotes; // Par de <tick, lane>

    std::string currentSection = "";

    while (std::getline(chartFile, line)) {
        // Remove espaços em branco
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        if (line[0] == '[' && line.back() == ']') {
            currentSection = line;
            continue;
        }

        if (currentSection == "[Song]") {
            std::stringstream ss(line);
            std::string key;
            ss >> key;
            if (key == "Resolution") {
                ss >> key; // Pula o "="
                ss >> resolution;
            }
        } else if (currentSection == "[SyncTrack]") {
            std::stringstream ss(line);
            int tick;
            char type;
            double bpm;
            ss >> tick >> type >> type; // Lê "tick = B"
            if (type == 'B') {
                ss >> bpm;
                bpmEvents.push_back({tick, bpm / 1000.0});
            }
        } else if (currentSection == "[ExpertSingle]") { // Ou outra dificuldade
            std::stringstream ss(line);
            int tick;
            char type;
            int lane;
            ss >> tick >> type >> type; // Lê "tick = N"
            if (type == 'N') {
                ss >> lane;
                if (lane >= 0 && lane <= 4) { // Ignora outros tipos de eventos
                    rawNotes.push_back({tick, lane});
                }
            }
        }
    }

    // --- Parte Crucial: Conversão de Ticks para Segundos ---
    std::vector<Note> finalNotes;
    if (bpmEvents.empty()) {
        // Adiciona um BPM padrão se nenhum for encontrado
        bpmEvents.push_back({0, 120.0});
    }

    double currentTime = 0.0;
    int lastTick = 0;
    int bpmEventIndex = 0;

    for (const auto& rawNote : rawNotes) {
        int noteTick = rawNote.first;

        // Avança o tempo até o tick da nota atual
        while (bpmEventIndex + 1 < bpmEvents.size() && bpmEvents[bpmEventIndex + 1].tick <= noteTick) {
            int ticksInSegment = bpmEvents[bpmEventIndex + 1].tick - lastTick;
            double secondsPerTick = 60.0 / (bpmEvents[bpmEventIndex].bpm * resolution);
            currentTime += ticksInSegment * secondsPerTick;
            lastTick = bpmEvents[bpmEventIndex + 1].tick;
            bpmEventIndex++;
        }

        // Calcula o tempo restante com o BPM atual
        int ticksRemaining = noteTick - lastTick;
        double secondsPerTick = 60.0 / (bpmEvents[bpmEventIndex].bpm * resolution);
        double noteTime = currentTime + (ticksRemaining * secondsPerTick);

        finalNotes.push_back({noteTime, rawNote.second, 0.0}); // Duração 0 por simplicidade
    }

    return finalNotes;
}