import React, {useEffect} from 'react';
import logo from './logo.svg';
import './App.css';
import {subscribePublishEvent} from "./utils/SocketIOUtils";

function App() {
    useEffect(() => {
        subscribePublishEvent((payload) => console.log(payload));
    }, []);
    return (
        <div className="App">
            <header className="App-header">
                This is very good example.
            </header>
        </div>
    );
}

export default App;
