import { Component, OnInit } from '@angular/core';

import { WebsocketDataService } from '../websocket-data.service';
import { RangeConvertExponential } from '../range-convert-exponential';

@Component({
  selector: 'app-ledtoggle',
  templateUrl: './ledtoggle.component.html',
  styleUrls: ['./ledtoggle.component.css']
})
export class LEDToggleComponent implements OnInit {
  public values = [];
  public mode: string;
  public pattern: string;
  public period: number;
  private sliderConvert: RangeConvertExponential;

  constructor(private websocketData: WebsocketDataService) {
    this.sliderConvert = new RangeConvertExponential(0, 100, 10, 2000);
    console.log('a = ' + this.sliderConvert.a);
    console.log('b = ' + this.sliderConvert.b);
  }

  handleData(message) {
    var json = JSON.parse(message);
    console.log(json);
    var state = json.state;
    if ('values' in state) {
      this.values = state.values;
    }
    
    if ('mode' in state) {
      this.mode = state.mode;
    }

    if ('pattern' in state) {
      this.pattern = state.pattern;
    }

    if ('period' in state) {
      this.period = state.period;
    }
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

  toggleLed(i: number) {
    var request = {
      type: "toggle",
      arg: {
        iled: i
      }
    };
    console.log("Toggling LED " + i);
    this.websocketData.socket.next(request);
  }

  setMode(mode: string): void {
    var request = {
      type: "setMode",
      arg: {
        mode: mode
      }
    }

    console.log("Setting mode: " + mode);
    this.websocketData.socket.next(request);
  }

  setPattern(pattern: string): void {
    var request = {
      type: "setPattern",
      arg: {
        pattern: pattern
      }
    }

    console.log("Setting patter: " + pattern);
    this.websocketData.socket.next(request);
  }

  setPeriod(sliderValue: string): void {
    var numValue = Number.parseInt(sliderValue);
    console.log("numValue = " + numValue);
    var newPeriod = this.sliderConvert.expConvert(numValue);
    console.log("newPeriod = " + newPeriod);

    var request = {
      type: "setPeriod",
      arg: {
        period: newPeriod
      }
    }

    console.log("Setting period: " + newPeriod);
    this.websocketData.socket.next(request);
  }
}
