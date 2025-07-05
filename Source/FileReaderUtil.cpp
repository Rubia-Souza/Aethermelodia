//
// Created by Aline on 20/06/2025.
//

#include "FileReaderUtil.h"
#include "SDL_log.h"

std::vector<Note> FileReaderUtil::loadChartManually(const std::string& filePath, Difficulty difficulty) {
    std::string difficultyString = getDifficultyString(difficulty);
    std::ifstream chartFile(filePath);
    std::string line;

    if (!chartFile.is_open()) {
        SDL_Log("Failed to load paths: %s", filePath.c_str());
        return {};
    }

    // 1 batida (semínima) = 192 ticks
    long resolution = 192;
    std::vector<BpmChangeEvent> bpmEvents;
    std::vector<RawNoteData> rawNotes;

    std::string currentSection = "";

    while (std::getline(chartFile, line)) {
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
                ss >> key;
                ss >> resolution;
            }
        } else if (currentSection == "[SyncTrack]") {
            std::stringstream ss(line);
            int tick;
            std::string type;
            double bpm;
            ss >> tick >> type >> type;
            if (type == "B") {
                ss >> bpm;
                bpmEvents.push_back({tick, bpm / 1000.0});
            }
        } else if (currentSection == difficultyString) {
            std::stringstream ss(line);
            int tick;
            std::string type;
            int lane;

            int durationTicks = 0;

            ss >> tick >> type >> type;
            if (type == "N") {
                ss >> lane >> durationTicks;
                if (lane >= 0 && lane <= 4) {
                    rawNotes.push_back({tick, lane, durationTicks});
                }
            }
        }
    }
    chartFile.close();

    std::vector<Note> finalNotes;
    if (bpmEvents.empty()) {
        bpmEvents.push_back({0, 120.0}); // BPM padrão
    }

    double currentTime = 0.0;
    int lastTick = 0;
    size_t bpmEventIndex = 0; // Usar size_t para compatibilidade com .size()

    for (const auto& rawNote : rawNotes) {
        int noteTick = rawNote.tick;

        while (bpmEventIndex + 1 < bpmEvents.size() && bpmEvents[bpmEventIndex + 1].tick <= noteTick) {
            int ticksInSegment = bpmEvents[bpmEventIndex + 1].tick - lastTick;
            double currentBpm = bpmEvents[bpmEventIndex].bpm;
            double secondsPerTick = 60.0 / (currentBpm * resolution);
            currentTime += ticksInSegment * secondsPerTick;
            lastTick = bpmEvents[bpmEventIndex + 1].tick;
            bpmEventIndex++;
        }

        double currentBpm = bpmEvents[bpmEventIndex].bpm;
        double secondsPerTick = 60.0 / (currentBpm * resolution);

        // Calcula o tempo de início da nota
        int ticksRemaining = noteTick - lastTick;
        double noteTime = currentTime + (ticksRemaining * secondsPerTick);

        double noteDurationSeconds = rawNote.durationTicks * secondsPerTick;

        finalNotes.push_back({noteTime, rawNote.lane, noteDurationSeconds});
    }

    return finalNotes;
};

std::string FileReaderUtil::getDifficultyString(Difficulty difficulty) {

    switch (difficulty) {
        case Difficulty::EASY_SINGLE:
            return "[EasySingle]";
        case Difficulty::MEDIUM_SINGLE:
            return "[MediumSingle]";
        case Difficulty::HARD_SINGLE:
            return "[HardSingle]";
        case Difficulty::EXPERT_SINGLE:
        default:
            return "[ExpertSingle]";
    }
}

std::string FileReaderUtil::getDifficultyName(Difficulty difficulty)
{
    switch (difficulty) {
        case Difficulty::EASY_SINGLE:
            return "Easy";
        case Difficulty::MEDIUM_SINGLE:
            return "Medium";
        case Difficulty::HARD_SINGLE:
            return "Hard";
        case Difficulty::EXPERT_SINGLE:
            return "Expert";
        default:
            return "Expert";
    }
}
