import { Component, OnInit } from '@angular/core';

import { WebsocketDataService } from '../websocket-data.service';

@Component({
  selector: 'app-ledtoggle',
  templateUrl: './ledtoggle.component.html',
  styleUrls: ['./ledtoggle.component.css']
})
export class LEDToggleComponent implements OnInit {
  public values = [];

  constructor(private websocketData: WebsocketDataService) { }

  handleData(message) {
    var json = JSON.parse(message);
    console.log(json);
    this.values = json.state.values;
  }

  ngOnInit(): void {
    this.websocketData.connect();
    var request = {type:'getState'};
    this.websocketData.socket.next(request);
    this.websocketData.socket.subscribe(
      msg => msg.text().then(text => this.handleData(text)),
      err => console.log(err)
    );
  }

  toggleLed(i: Number) {
    var request = {
      type: "toggle",
      arg: {
        iled: i
      }
    };
    console.log("Toggling LED " + i);
    this.websocketData.socket.next(request);
  }
}
