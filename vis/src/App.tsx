import React, {useEffect, useState} from 'react';

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

interface HistoryItem {
    t: number
    score: number
}

function App() {
    const [context, setContext] = useState<CanvasRenderingContext2D | null>(null);
    const [payload, setPayload] = useState<DrawPayload | null>();
    const [selectedRectangles, setSelectedRectangles] = useState<Rect[]>([]);
    const [communicationFile, setCommunicationFile] = useState<string | null>();

    const [history, setHistory] = useState<HistoryItem[]>([]);

    useEffect(() => {
        const canvas = document.getElementById("canvas") as HTMLCanvasElement | null;
        const canvasContext = canvas?.getContext("2d") ?? null;
        setContext(canvasContext)
    }, []);

    const [startPoint, setStartPoint] = useState<Point | null>(null);
    const [endPoint, setEndPoint] = useState<Point | null>(null);

    useEffect(() => {
        if (!context || !payload) return;
        context.fillStyle = "white";
        context.fillRect(0, 0, context.canvas.width, context.canvas.height);

        payload.rects.forEach(
            (r_) => {
                const rect = {...r_};
                let val = rect.need / 1000000;
                context.fillStyle = scoreToColor(rect.subScore, val);

                if (!(rect.l <= rect.px && rect.px < rect.r) || !(rect.d <= rect.py && rect.py < rect.u)) {
                    context.fillStyle = "gray";
                }

                // context.fillStyle = contributionColor(r.subScore, payload.score);
                rect.l /= 10;
                rect.r /= 10;
                rect.d /= 10;
                rect.u /= 10;
                rect.px /= 10;
                rect.py /= 10;

                context.fillRect(rect.l, rect.d, rect.r - rect.l, rect.u - rect.d);
                context.strokeStyle = "black";
                context.strokeRect(rect.l, rect.d, rect.r - rect.l, rect.u - rect.d);


                if (selectedRectangles.some((s) => s.id == rect.id)) {
                    context.fillStyle = "rgba(255, 0, 255, 0.5)";
                    context.fillRect(rect.l, rect.d, rect.r - rect.l, rect.u - rect.d);
                }

                context.beginPath();
                context.arc(rect.px, rect.py, 2, 0, 2 * Math.PI);
                context.stroke();

                const qx = (rect.l + rect.r) / 2;
                const qy = (rect.u + rect.d) / 2;

                context.beginPath();
                context.moveTo(qx, qy);
                context.lineTo(rect.px, rect.py);
                context.stroke();

                context.fillStyle = "black";
                context.fillText(`${Math.round(rect.subScore * 100) / 100}`, qx, qy + 10);
                context.fillText(`${rect.need}`, qx, qy);
            }
        );
        if (startPoint && endPoint) {
            context.fillStyle = "rgba(128, 128, 255, 0.5)";
            context.fillRect(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y);
        }


    }, [payload, selectedRectangles, startPoint, endPoint]);

    useEffect(() => {
        subscribePublishEvent((payload) => {
            if (payload.type == "draw") {
                setPayload(payload);
                history.push({t: payload.relTime, score: payload.score})
                setHistory(history);
            } else if (payload.type == "communication") {
                setCommunicationFile(payload.file);
            } else {
                setHistory([]);
            }
        });
    }, []);
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
                {!context && "Loading Canvas..."}
                <p>{payload?.score.toFixed(6)}</p>
                <canvas width="1000" height="1000" id="canvas"
                        onMouseDown={(e) => {
                            if (!context) return;
                            const X = e.clientX - context.canvas.getBoundingClientRect().left;
                            const Y = e.clientY - context.canvas.getBoundingClientRect().top;
                            setStartPoint({
                                x: X,
                                y: Y
                            });
                        }}

                        onMouseMove={(e) => {
                            if (!context) return;

                            const X = e.clientX - context.canvas.getBoundingClientRect().left;
                            const Y = e.clientY - context.canvas.getBoundingClientRect().top;
                            if (startPoint != null) {
                                setEndPoint({
                                    x: X,
                                    y: Y
                                })
                            }
                        }}

                        onMouseUp={async (e) => {
                            if (!context) return;

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
                <div className={"some-info"}>
                    Data points: {history.length}
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
