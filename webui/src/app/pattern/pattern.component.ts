import { Component, OnInit, OnDestroy } from '@angular/core';
import { Subscription } from 'rxjs';

import { RangeConvertExponential } from '../range-convert-exponential';
import { StateService } from '../state.service';

@Component({
  selector: 'app-pattern',
  templateUrl: './pattern.component.html',
  styleUrls: ['./pattern.component.css']
})
export class PatternComponent implements OnInit, OnDestroy {
  public pattern: string;
  public period: number;
  private sliderConvert: RangeConvertExponential;

  private patternSub: Subscription;
  private periodSub: Subscription;

  constructor(private stateService: StateService) {
    this.sliderConvert = new RangeConvertExponential(0, 100, 10, 2000);
  }

  ngOnInit(): void {
    this.patternSub = this.stateService.subscribeToPattern(this.updatePattern(), err => console.log(err));
    this.periodSub = this.stateService.subscribeToPeriod(this.updatePeriod(), err => console.log(err));
  }

  ngOnDestroy(): void {
    this.patternSub.unsubscribe();
    this.periodSub.unsubscribe();
  }

  updatePattern() {
    var comp = this;

    function update(pattern: string): void {
      comp.pattern = pattern;
    }

    return update;
  }
  updatePeriod() {
    var comp = this;

    function update(period: number): void {
      comp.period = period;
    }
    
    return update;
  }

  // request functions
  setPattern(pattern: string): void { this.stateService.setPattern(pattern); }
  setPeriod(rawSliderValue: string): void { 
    var period = this.sliderConvert.expConvert(Number.parseInt(rawSliderValue));
    this.stateService.setPeriod(period);
  }
}
