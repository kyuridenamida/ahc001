export interface Rect {
    l: number,
    r: number,
    u: number,
    d: number,
    px: number,
    py: number,
    id: number,
    need: number,
    subScore: number,
}

export interface DrawPayload {
    type: "draw"
    rects: Rect[]
    score: number,
    fakeScore: number,
    relTime: number,
    temperature: number,
}

export interface CommunicationRegisterPayload {
    type: "communication"
    file: string
}
export interface ResetPayload {
    type: "reset"
}

export type Payload = DrawPayload | CommunicationRegisterPayload | ResetPayload;
