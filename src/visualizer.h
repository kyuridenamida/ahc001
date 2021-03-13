//
// Created by kyuridenamida on 2021/03/13.
//

#ifndef AHC001_VISUALIZER_H
#define AHC001_VISUALIZER_H


#include <functional>
#include "timer.h"
#include "./httputils.h"

class Visualizer {
    RealTimer *timer;
    const string SEND_JSON_END_POINT = "http://localhost:8888/json/";
public:
    Visualizer(RealTimer *realTimer) : timer(realTimer) {
    }


    void emitJson(const string &jsonString) const {
        HttpUtils::emitJsonToUrl(SEND_JSON_END_POINT, jsonString);
    }

    void emitJsonWithTimer(std::function<string(void)> lazyJsonSupplier) {
        double now = timer->time_elapsed();
        if (now - lastCommunicationTime > 0.016) {
            // 60 rps
            emitJson(lazyJsonSupplier());
            lastCommunicationTime = now;
        }
    }

    double lastCommunicationTime = -99999;
};

template<class VisComResponse>
class VisualizerCommunicator {
    RealTimer *timer;
public:
    VisualizerCommunicator(
            string communicationFile,
            Visualizer *visualizer,
            RealTimer *timer
    ) : communicationFile(std::move(communicationFile)),
        visualizer(visualizer),
        timer(timer) {}

    static VisualizerCommunicator *start(Visualizer *visualizer, RealTimer *timer) {
        const int randomNumber = (int) (std::random_device()() % 100000);
        const string communicationFile = "/tmp/" + to_string(randomNumber) + ".com";
        auto *visualizerCommunicator =
                new VisualizerCommunicator(communicationFile, visualizer, timer);

        visualizerCommunicator->sendRegisterFileToVisualizer();

        return visualizerCommunicator;
    }

    void sendRegisterFileToVisualizer() {
        visualizer->emitJson(HttpUtils::mapToJson(
                {
                        {"type", HttpUtils::jsonValue("communication")},
                        {"file", HttpUtils::jsonValue(communicationFile)},
                }));
    }

    VisComResponse receiveResponseIfExists() {
#ifndef CLION
        return VisComResponse::empty();
#endif
        double now = timer->time_elapsed();
        if (now - lastCommunicationTime > 0.2) {
            //一応ファイルを再度Visualizerに登録する。
            sendRegisterFileToVisualizer();
            VisComResponse response = readResponseFromFileIfExists();
            lastCommunicationTime = now;
            return response;
        }
        return VisComResponse::empty();
    }

private:
    const string communicationFile;
    const Visualizer *visualizer;
    double lastCommunicationTime = -99999;

    VisComResponse readResponseFromFileIfExists() {
        ifstream ifs(communicationFile);
        if (!ifs.is_open()) {
            cerr << "Communicating but no file found yet." << endl;
            return VisComResponse::empty();
        }
        cerr << "Communicating and found the file!" << endl;
        auto resp = VisComResponse::readFromStream(ifs);

        // Remove the file
        remove(communicationFile.c_str());
        return resp;
    }
};

#endif //AHC001_VISUALIZER_H
