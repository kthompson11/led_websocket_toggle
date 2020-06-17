import { Injectable } from '@angular/core';
import { WebsocketDataService } from './websocket-data.service';
import { Subscription, BehaviorSubject } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class StateService {
  private socket: Subscription;

  // state variables
  private mode: string;
  private values: number[];
  private pattern: string;
  private period: number;

  // subscribable 
  public modeSub: BehaviorSubject<string>;
  private valuesSub: BehaviorSubject<number[]>;
  private patternSub: BehaviorSubject<string>;
  private periodSub: BehaviorSubject<number>;

  constructor(private websocketData: WebsocketDataService) {
    this.websocketData.connect();
    this.socket = websocketData.socket.subscribe(
      data => data.text().then(t => this.parseData(t)),
      err => console.log(err)
    );

    this.modeSub = new BehaviorSubject<string>('');
    this.valuesSub = new BehaviorSubject<number[]>([]);
    this.patternSub = new BehaviorSubject<string>('');
    this.periodSub = new BehaviorSubject<number>(0);

    this.requestState();
  }

  parseData(data: any): void {
    var json = JSON.parse(data);
    console.log(json);
    var state = json.state;
    if ('mode' in state) {
      this.mode = state.mode;
      this.modeSub.next(this.mode);
    }
    if ('values' in state) {
      this.values = state.values;
      this.valuesSub.next(this.values);
    }
    if ('pattern' in state) {
      this.pattern = state.pattern;
      this.patternSub.next(this.pattern);
    }
    if ('period' in state) {
      this.period = state.period;
      this.periodSub.next(this.period);
    }
  }

  subscribeToMode(next?: (value: string) => void, error?: (error: any) => void, complete?: () => void): Subscription {
    return this.modeSub.subscribe(next, error, complete);
  }
  subscribeToValues(next?: (value: number[]) => void, error?: (error: any) => void, complete?: () => void): Subscription {
    return this.valuesSub.subscribe(next, error, complete);
  }
  subscribeToPattern(next?: (value: string) => void, error?: (error: any) => void, complete?: () => void): Subscription {
    return this.patternSub.subscribe(next, error, complete);
  }
  subscribeToPeriod(next?: (value: number) => void, error?: (error: any) => void, complete?: () => void): Subscription {
    return this.periodSub.subscribe(next, error, complete);
  }

  // Requests
  requestState(): void {
    var request = {
      type: 'getState'
    };
    console.log("Getting state");
    console.log(JSON.stringify(request));
    this.websocketData.socket.next(request);
  }

  setMode(mode: string): void {
    var request = {
      type: "setMode",
      arg: {
        mode: mode
      }
    };
    console.log("Setting mode: " + mode);
    this.websocketData.socket.next(request);
  }

  toggleLED(i: number): void {
    var request = {
      type: "toggle",
      arg: {
        iled: i
      }
    };
    console.log("Toggling LED " + i);
    this.websocketData.socket.next(request);
  }

  setPattern(pattern: string): void {
    var request = {
      type: "setPattern",
      arg: {
        pattern: pattern
      }
    }
    console.log("Setting pattern: " + pattern);
    this.websocketData.socket.next(request);
  }

  setPeriod(period: number): void {
    var request = {
      type: "setPeriod",
      arg: {
        period: period
      }
    }
    console.log("Setting period: " + period);
    this.websocketData.socket.next(request);
  }

}
