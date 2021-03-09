import {io, Socket} from 'socket.io-client';
import {Payload} from "../models/Payload";

const socket = io();
export const subscribePublishEvent = (callback: (payload: Payload) => void) => {
    socket.on("publish", (payload: Payload) => callback(payload));
}
