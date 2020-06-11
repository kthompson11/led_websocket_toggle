import { Injectable } from '@angular/core';
import { webSocket, WebSocketSubject } from 'rxjs/webSocket'

@Injectable({
  providedIn: 'root'
})
export class WebsocketDataService {
  public socket: WebSocketSubject<any>;

  public connect() {
    if ((this.socket === undefined) || (this.socket.closed === true)) {
      this.socket = webSocket({
        url: "ws://192.168.1.145:8020",
        deserializer: ({data}) => data
      });
    }
  }

  public close() {
    if (this.socket !== undefined) {
      this.socket.complete();
    }
  }

  constructor() { }
}
