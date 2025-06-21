//
// Created by Aline on 20/06/2025.
//

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

// Estrutura para uma nota no jogo
struct Note {
    double timeInSeconds; // O tempo da nota em segundos desde o início da música
    int lane;             // Trilha/Cor da nota (0-4)
    double durationInSeconds; // Duração em segundos (para notas longas)
};

// Estrutura para um evento de mudança de BPM
struct BpmChangeEvent {
    int tick;
    double bpm;
};

class FileReaderUtil
{
    public:
        static std::vector<Note> loadChartManually(const std::string& filePath);
};