import React, {useEffect, useMemo, useState} from 'react';

import './App.css';
import {subscribePublishEvent} from "./utils/SocketIOUtils";
import {DrawPayload, Rect} from "./models/Payload";

const contributionColor = (subScore: number, overallScore: number) => {
    const d = subScore - overallScore;
    if (Math.abs(d) <= 0.0) {
        return "rgba(255 , 255, 255, 1)";
    } else if (d < 0) {
        return "rgba(255, 64, 64, 1)";
    } else {
        return "rgba(128, 255, 64, 1)"
    }
}

const scoreToColor = (score: number, trans: number) => {
    if (score <= 0.5) {
        return `rgba(255, 64, 64, ${trans})`;
    } else if (score <= 0.9) {
        return `rgba(255, 128, 64, ${trans})`;
    } else if (score <= 0.98) {
        return `rgba(192, 192, 64, ${trans})`;
    } else if (score <= 0.99) {
        return `rgba(128, 255, 64, ${trans})`;
    } else {
        return `rgba(0, 128, 255, ${trans})`;
    }
}

interface Point {
    x: number;
    y: number;
}

interface TimeScorePair {
    t: number
    score: number
}


interface TimeTemperaturePair {
    t: number
    temperature: number
}

function App() {
    const [scoreChartContext, setScoreChartContext] = useState<CanvasRenderingContext2D | null>(null);
    const [mainChartContext, setMainChartContext] = useState<CanvasRenderingContext2D | null>(null);
    const [temperatureChartContext, setTemperatureChartContext] = useState<CanvasRenderingContext2D | null>(null);

    const [payload, setPayload] = useState<DrawPayload | null>();
    const [selectedRectangles, setSelectedRectangles] = useState<Rect[]>([]);
    const [communicationFile, setCommunicationFile] = useState<string | null>();

    useEffect(() => {
        const mainChartCanvas = document.getElementById("mainCanvas") as HTMLCanvasElement | null;
        const mainChartCanvasContext = mainChartCanvas?.getContext("2d") ?? null;
        setMainChartContext(mainChartCanvasContext)

        const scoreChartCanvas = document.getElementById("scoreCanvas") as HTMLCanvasElement | null;
        const scoreChartContext = scoreChartCanvas?.getContext("2d") ?? null;
        setScoreChartContext(scoreChartContext);
        if (scoreChartContext) {
            scoreChartContext.fillStyle = "lightgray";
            scoreChartContext.fillRect(0, 0, scoreChartContext.canvas.width, scoreChartContext.canvas.height);
        }

        const temperatureChartContextCanvas = document.getElementById("temperatureCanvas") as HTMLCanvasElement | null;
        const temperatureChartContextContext = temperatureChartContextCanvas?.getContext("2d") ?? null;
        setTemperatureChartContext(temperatureChartContextContext);
        if (temperatureChartContextContext) {
            temperatureChartContextContext.fillStyle = "lightgray";
            temperatureChartContextContext.fillRect(0, 0, temperatureChartContextContext.canvas.width, temperatureChartContextContext.canvas.height);
        }

    }, []);

    const [startPoint, setStartPoint] = useState<Point | null>(null);
    const [endPoint, setEndPoint] = useState<Point | null>(null);

    useEffect(() => {
        if (!mainChartContext || !payload) return;
        mainChartContext.fillStyle = "white";
        mainChartContext.fillRect(0, 0, mainChartContext.canvas.width, mainChartContext.canvas.height);

        const xScale =  mainChartContext.canvas.width / 10000;
        const yScale =  mainChartContext.canvas.height / 10000;
        payload.rects.forEach(
            (r_) => {
                const rect = {...r_};
                let val = rect.need / 1000000;
                mainChartContext.fillStyle = scoreToColor(rect.subScore, val);

                if (!(rect.l <= rect.px && rect.px < rect.r) || !(rect.d <= rect.py && rect.py < rect.u)) {
                    mainChartContext.fillStyle = "gray";
                }
                rect.l *= xScale;
                rect.r *= xScale;
                rect.d *= yScale;
                rect.u *= yScale;
                rect.px *=xScale;
                rect.py *= yScale;

                mainChartContext.fillRect(rect.l, rect.d, rect.r - rect.l, rect.u - rect.d);
                mainChartContext.strokeStyle = "black";
                mainChartContext.strokeRect(rect.l, rect.d, rect.r - rect.l, rect.u - rect.d);


                if (selectedRectangles.some((s) => s.id == rect.id)) {
                    mainChartContext.fillStyle = "rgba(255, 0, 255, 0.5)";
                    mainChartContext.fillRect(rect.l, rect.d, rect.r - rect.l, rect.u - rect.d);
                }

                mainChartContext.beginPath();
                mainChartContext.arc(rect.px, rect.py, 2, 0, 2 * Math.PI);
                mainChartContext.stroke();

                const qx = (rect.l + rect.r) / 2;
                const qy = (rect.u + rect.d) / 2;

                mainChartContext.beginPath();
                mainChartContext.moveTo(qx, qy);
                mainChartContext.lineTo(rect.px, rect.py);
                mainChartContext.stroke();

                mainChartContext.fillStyle = "black";
                mainChartContext.fillText(`${Math.round(rect.subScore * 100) / 100}`, qx, qy + 10);
                mainChartContext.fillText(`${rect.need}`, qx, qy);
            }
        );
        if (startPoint && endPoint) {
            mainChartContext.fillStyle = "rgba(128, 128, 255, 0.5)";
            mainChartContext.fillRect(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y);
        }


    }, [payload, selectedRectangles, startPoint, endPoint]);

    const clearCanvas = () => {
        if (scoreChartContext) {
            scoreChartContext.fillStyle = "lightgray";
            scoreChartContext.fillRect(0, 0, scoreChartContext.canvas.width, scoreChartContext.canvas.height);
            scoreChartContext.beginPath();
        }
        if (mainChartContext) {
            mainChartContext.fillStyle = "white";
            mainChartContext.fillRect(0, 0, mainChartContext.canvas.width, mainChartContext.canvas.height);
        }

        if (temperatureChartContext) {
            temperatureChartContext.fillStyle = "lightgray";
            temperatureChartContext.fillRect(0, 0, temperatureChartContext.canvas.width, temperatureChartContext.canvas.height);
            temperatureChartContext.beginPath();
        }
    }

    const addPointToScoreChart = (point: TimeScorePair) => {
        if (!scoreChartContext) return;
        scoreChartContext.strokeStyle = "blue";
        scoreChartContext.lineTo(
            point.t * scoreChartContext.canvas.width,
            scoreChartContext.canvas.height - point.score * scoreChartContext.canvas.height
        );
        scoreChartContext.stroke();
    };

    const addPointToTemperatureChart = (point: TimeTemperaturePair) => {
        if (!temperatureChartContext) return;
        temperatureChartContext.strokeStyle = "red";
        const startTemperature = 0.0005;
        temperatureChartContext.lineTo(point.t * temperatureChartContext.canvas.width,
            temperatureChartContext.canvas.height - point.temperature * temperatureChartContext.canvas.height * (1. / startTemperature));
        temperatureChartContext.stroke();
    };
    useEffect(() => {
        subscribePublishEvent((payload) => {
            if (payload.type == "draw") {
                setPayload(payload);
                addPointToScoreChart({t: payload.relTime, score: payload.score});
                addPointToTemperatureChart({t: payload.relTime, temperature: payload.temperature})
            } else if (payload.type == "communication") {
                setCommunicationFile(payload.file);
            } else {
                clearCanvas();
            }
        });
    }, [scoreChartContext, mainChartContext, temperatureChartContext]);
    const sendRemoveRequest = async (file: string, remIndexes: number[]) => {
        await fetch("/write/", {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                file: file,
                fileContent: remIndexes.map(x => `${x}`).join(" ")
            })
        });
    };
    return (
        <div className="App">
            <div className="App-main">
                {!mainChartContext && "Loading Canvas..."}

                <p>{payload?.temperature.toFixed(6)} (Temperature) </p>
                <canvas width="1000" height="100" id="temperatureCanvas"/>

                <p>{payload?.score.toFixed(6)} (Score)</p>
                <canvas width="1000" height="200" id="scoreCanvas"/>

                <canvas width="1000" height="1000" id="mainCanvas"
                        onMouseDown={(e) => {
                            if (!mainChartContext) return;
                            const X = e.clientX - mainChartContext.canvas.getBoundingClientRect().left;
                            const Y = e.clientY - mainChartContext.canvas.getBoundingClientRect().top;
                            setStartPoint({
                                x: X,
                                y: Y
                            });
                        }}

                        onMouseMove={(e) => {
                            if (!mainChartContext) return;

                            const X = e.clientX - mainChartContext.canvas.getBoundingClientRect().left;
                            const Y = e.clientY - mainChartContext.canvas.getBoundingClientRect().top;
                            if (startPoint != null) {
                                setEndPoint({
                                    x: X,
                                    y: Y
                                })
                            }
                        }}

                        onMouseUp={async (e) => {
                            if (!mainChartContext) return;

                            if (startPoint && endPoint) {
                                const selectedRects = (payload?.rects ?? [])
                                    .filter(r_ => {
                                        const r = {...r_};
                                        r.l /= 10;
                                        r.r /= 10;
                                        r.d /= 10;
                                        r.u /= 10;
                                        const L = Math.min(startPoint.x, endPoint.x);
                                        const R = Math.max(startPoint.x, endPoint.x);
                                        const D = Math.min(startPoint.y, endPoint.y);
                                        const U = Math.max(startPoint.y, endPoint.y);
                                        if (Math.min(r.r, R) - Math.max(r.l, L) > 0) {
                                            if (Math.min(r.u, U) - Math.max(r.d, D) > 0) {
                                                return true;
                                            }
                                        }
                                        return false;
                                    });
                                if (communicationFile != null) {
                                    sendRemoveRequest(communicationFile, selectedRects.map(r => r.id));
                                    setSelectedRectangles(selectedRects);
                                }
                            }
                            setStartPoint(null);
                            setEndPoint(null);
                        }}
                />
                <div className={"some-info"}>
                    Communication File: {communicationFile}
                </div>
                <div className={"selected-rects"}>
                    <p>Selected IDs:</p>
                    <p>{selectedRectangles.map(r => r.id).join(",")}</p>
                </div>
            </div>
        </div>
    );
}

export default App;
