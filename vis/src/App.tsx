import React, {useEffect, useState} from 'react';

import logo from './logo.svg';
import './App.css';
import {subscribePublishEvent} from "./utils/SocketIOUtils";
import {Payload} from "./models/Payload";

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
    if (score <= 0.2) {
        return "rgba(255, 64, 64, 1)";
    } else if (score <= 0.4) {
        return "rgba(255, 128, 64, 1)";
    } else if (score <= 0.6) {
        return "rgba(192, 192, 64, 1)";
    } else if (score <= 0.8) {
        return "rgba(128, 255, 64, 1)";
    } else {
        return "rgba(0, 128, 255, 1)";
    }
}

function App() {
    const [context, setContext] = useState<CanvasRenderingContext2D | null>(null);
    const [payload, setPayload] = useState<Payload | null>();

    useEffect(() => {
        const canvas = document.getElementById("canvas") as HTMLCanvasElement | null;
        const canvasContext = canvas?.getContext("2d") ?? null;
        setContext(canvasContext)
    }, []);

    useEffect(() => {
        if (!context || !payload) return;
        context.fillStyle = "white";
        context.fillRect(0, 0, context.canvas.width, context.canvas.height);

        payload.rects.forEach(
            (r_) => {
                const r = {...r_};
                // context.fillStyle = scoreToColor(r.subScore);
                context.fillStyle = contributionColor(r.subScore, payload.score);
                r.l /= 10;
                r.r /= 10;
                r.d /= 10;
                r.u /= 10;
                r.px /= 10;
                r.py /= 10;
                context.fillRect(r.l, r.d, r.r - r.l, r.u - r.d);
                console.log(r.l, r.d);
                context.strokeStyle = "black";
                context.strokeRect(r.l, r.d, r.r - r.l, r.u - r.d);

                context.beginPath();
                context.arc(r.px, r.py, 2, 0, 2 * Math.PI);
                context.stroke();

                const qx = (r.l + r.r) / 2;
                const qy = (r.u + r.d) / 2;

                context.beginPath();
                context.moveTo(qx, qy);
                context.lineTo(r.px, r.py)
                context.stroke();
            }
        )

    }, [payload]);

    useEffect(() => {
        subscribePublishEvent((payload) => {
            setPayload(payload);
        });
    }, []);
    return (
        <div className="App">
            <header className="App-header">
                {!context && "Loading Canvas..."}
                {payload?.score}
                <canvas width="1000" height="1000" id="canvas"/>
            </header>
        </div>
    );
}

export default App;
