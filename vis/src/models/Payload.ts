export interface Rect {
    l: number,
    r: number,
    u: number,
    d: number,
    px: number,
    py: number,
    id: number,
    need: number,
    subScore: number
}

export interface Payload {
    rects: Rect[]
    score: number
}
