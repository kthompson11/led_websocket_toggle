import { Component, OnInit, OnDestroy } from '@angular/core';
import { UIRouter } from '@uirouter/angular';
import { Subscription } from 'rxjs';

import { StateService } from '../state.service';

@Component({
  selector: 'app-ledtoggle',
  templateUrl: './ledtoggle.component.html',
  styleUrls: ['./ledtoggle.component.css']
})
export class LEDToggleComponent implements OnInit, OnDestroy {
  
  public mode: string;
  private modeSub: Subscription;
  
  constructor(
    private stateService: StateService,
    private router: UIRouter
  ) {}

  ngOnInit(): void {
    this.modeSub = this.stateService.modeSub.subscribe(this.updateMode(), err => console.log(err));
  }

  ngOnDestroy(): void {
    this.modeSub.unsubscribe();
  }

  updateMode() {
    var comp = this;
    
    function update(mode: string): void {
      comp.mode = mode;
      if (comp.router !== undefined) {
        comp.router.stateService.go(mode, undefined, {location: 'false'});
      } else {
        console.log('uirouter undefined');
      }
      console.log("mode = " + comp.mode);
    }

    return update;
  }

  setMode(mode: string): void { 
    this.stateService.setMode(mode); 
  }
}
