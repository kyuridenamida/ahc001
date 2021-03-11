import React, {useEffect, useMemo, useState} from 'react';

import logo from './logo.svg';
import './App.css';
import {subscribePublishEvent} from "./utils/SocketIOUtils";
import {DrawPayload, Payload, Rect} from "./models/Payload";

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

const scoreToColor = (score: number) => {
    if (score <= 0.5) {
        return "rgba(255, 64, 64, 1)";
    } else if (score <= 0.9) {
        return "rgba(255, 128, 64, 1)";
    } else if (score <= 0.95) {
        return "rgba(192, 192, 64, 1)";
    } else if (score <= 0.991) {
        return "rgba(128, 255, 64, 1)";
    } else {
        return "rgba(0, 128, 255, 1)";
    }
}

interface Point {
    x: number;
    y: number;
}

function App() {
    const [context, setContext] = useState<CanvasRenderingContext2D | null>(null);
    const [payload, setPayload] = useState<DrawPayload | null>();
    const [selectedRectangles, setSelectedRectangles] = useState<Rect[]>([]);
    const [communicationFile, setCommunicationFile] = useState<string | null>();

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
                context.fillStyle = scoreToColor(rect.subScore);

                if (!(rect.l <= rect.px && rect.px < rect.r) || !(rect.d <= rect.py && rect.py < rect.u)) {
                    context.fillStyle = "gray";
                }

                if (selectedRectangles.some((s) => s.id == rect.id)) {
                    context.fillStyle = "rgba(255, 0, 255, 1)";
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

                context.beginPath();
                context.arc(rect.px, rect.py, 2, 0, 2 * Math.PI);
                context.stroke();

                const qx = (rect.l + rect.r) / 2;
                const qy = (rect.u + rect.d) / 2;

                context.beginPath();
                context.moveTo(qx, qy);
                context.lineTo(rect.px, rect.py)
                context.stroke();

                context.fillStyle = "black";
                context.fillText(`${Math.round(rect.subScore * 100) / 100}`, qx, qy);
            }
        );
        if (startPoint && endPoint) {
            context.fillStyle = "rgba(128, 128, 255, 0.5)";
            context.fillRect(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y);
        }

        // payload.rects.forEach(
        //     (r_) => {
        //         const rect = {...r_};
        //         rect.l /= 10;
        //         rect.r /= 10;
        //         rect.d /= 10;
        //         rect.u /= 10;
        //         rect.px /= 10;
        //         rect.py /= 10;
        //
        //         selectedRectangles.forEach(s_ => {
        //             if (s_.id == r_.id) return;
        //
        //             const s = {...payload.rects[s_.id]};
        //             s.l /= 10;
        //             s.r /= 10;
        //             s.d /= 10;
        //             s.u /= 10;
        //             s.px /= 10;
        //             s.py /= 10;
        //             if (rect.px <= s.px) {
        //                 if (rect.py <= s.py) {
        //                     context.fillStyle = "rgba(0, 0, 0, 1)";
        //                     context.fillRect(0, 0, rect.px, rect.py);
        //                 } else {
        //                     context.fillStyle = "rgba(0, 0, 0, 1)";
        //                     context.fillRect(0, rect.py, rect.px, 1000 - rect.py);
        //                 }
        //             } else {
        //                 if (rect.py <= s.py) {
        //                     context.fillStyle = "rgba(0, 0, 0, 1)";
        //                     context.fillRect(rect.px, 0, 1000 - rect.px, rect.py);
        //                 } else {
        //                     context.fillStyle = "rgba(0, 0, 0, 1)";
        //                     context.fillRect(rect.px, rect.py, 1000 - rect.px, 1000 - rect.py);
        //
        //                 }
        //             }
        //         });
        //     }
        // );

        // selectedRectangles.forEach(s_ => {
        //     const s = {...s_};
        //     context.fillStyle = "red";
        //     context.strokeStyle = "blue";
        //     s.px /= 10;
        //     s.py /= 10;
        //     context.beginPath();
        //     context.moveTo(s.px, 0);
        //     context.lineTo(s.px, 1000);
        //     context.stroke();
        //     context.beginPath();
        //     context.moveTo(0, s.py);
        //     context.lineTo(1000, s.py);
        //     context.stroke();
        // });


    }, [payload, selectedRectangles, startPoint, endPoint]);

    useEffect(() => {
        subscribePublishEvent((payload) => {
            if (payload.type == "draw") {
                setPayload(payload);
            } else {
                setCommunicationFile(payload.file);
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
                remIndexes: remIndexes
            })
        });
    };
    return (
        <div className="App">
            <header className="App-header">
                {!context && "Loading Canvas..."}
                <p>{payload?.score}</p>
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
                                const selectedPoints = (payload?.rects ?? [])
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
                                        if( Math.min(r.r, R) - Math.max(r.l, L) > 0 ){
                                            if( Math.min(r.u, U) - Math.max(r.d, D) > 0 ){
                                                return true;
                                            }
                                        }
                                        return false;
                                    });
                                if (communicationFile != null) {
                                    sendRemoveRequest(communicationFile, selectedPoints.map(r => r.id));
                                }
                            }
                            setStartPoint(null);
                            setEndPoint(null);
                        }}

                        onDoubleClick={async (e) => {
                            if (!context) return;
                            const X = e.clientX - context.canvas.getBoundingClientRect().left;
                            const Y = e.clientY - context.canvas.getBoundingClientRect().top;
                            const selectedRects = (payload?.rects ?? [])
                                .filter(r => {
                                    if (r.l / 10 - 1 <= X && X <= r.r / 10 + 1) {
                                        if (r.d / 10 - 1 <= Y && Y <= r.u / 10 + 1) {
                                            return true;
                                        }
                                    }
                                    return false;
                                });
                            setSelectedRectangles(selectedRects);
                        }}
                        // onClick={async (e) => {
                        //     if (!context) return;
                        //     const X = e.clientX - context.canvas.getBoundingClientRect().left;
                        //     const Y = e.clientY - context.canvas.getBoundingClientRect().top;
                        //     const selectedRects = (payload?.rects ?? [])
                        //         .filter(r => {
                        //             if (r.l / 10 - 1 <= X && X <= r.r / 10 + 1) {
                        //                 if (r.d / 10 - 1 <= Y && Y <= r.u / 10 + 1) {
                        //                     return true;
                        //                 }
                        //             }
                        //             return false;
                        //         });
                        //     if (communicationFile != null) {
                        //         await sendRemoveRequest(communicationFile, selectedRects.map(r => r.id));
                        //     }
                        // }}
                />
                <p> Communication File: {communicationFile}</p>
                <p> Clicked size: {selectedRectangles.length}</p>
                <ul>
                    {selectedRectangles.map(r_ => {
                        const r = payload?.rects[r_.id];
                        if (!r) return <li></li>;
                        const area = (r.r - r.l) * (r.u - r.d);
                        const h = 1 - Math.min(1.0, (area / r.need));
                        return <li>id=[{r.id}] score=[{Math.round((1 - h * h) * 100) / 100}]</li>
                    })}
                </ul>
            </header>
        </div>
    );
}

export default App;
