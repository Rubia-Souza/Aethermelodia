//
// Created by Aline on 20/06/2025.
//

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

struct Note {
    double timeInSeconds; // O tempo da nota em segundos desde o início da música
    int lane;             // Trilha/Cor da nota (0-4)
    double durationInSeconds; // Duração em segundos (para notas longas)
};

struct BpmChangeEvent {
    int tick;
    double bpm;
};

struct RawNoteData {
    int tick;
    int lane;
    int durationTicks;
};
enum class Difficulty {
    EASY_SINGLE = 1,
    MEDIUM_SINGLE,
    HARD_SINGLE ,
    EXPERT_SINGLE,
};


class FileReaderUtil
{
    public:
        static std::string getDifficultyString(Difficulty difficulty);

        static std::string getDifficultyName(Difficulty difficulty);

        static std::vector<Note> loadChartManually(const std::string& filePath, Difficulty difficulty = Difficulty::EXPERT_SINGLE);
};