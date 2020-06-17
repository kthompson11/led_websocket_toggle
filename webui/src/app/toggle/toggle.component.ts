import { Component, OnInit, OnDestroy } from '@angular/core';

import { Subscription } from 'rxjs';
import { StateService } from '../state.service';
import { compileComponentFromMetadata } from '@angular/compiler';

@Component({
  selector: 'app-toggle',
  templateUrl: './toggle.component.html',
  styleUrls: ['./toggle.component.css']
})
export class ToggleComponent implements OnInit, OnDestroy {
  public values = [];
  private valueSub: Subscription;

  constructor(private stateService: StateService) {}

  ngOnInit(): void {
    this.valueSub = this.stateService.subscribeToValues(this.updateValues(), err => console.log(err));
  }

  ngOnDestroy(): void {
    this.valueSub.unsubscribe();
  }

  updateValues() {
    var comp = this;

    function update(values: number[]): void {
      comp.values = values;
    }

    return update;
  }

  // request functions
  toggleLed(i: number): void { this.stateService.toggleLED(i); }

}
