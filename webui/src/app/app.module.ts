import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';

import { UIRouterModule } from "@uirouter/angular";

import { AppComponent } from './app.component';
import { LEDToggleComponent } from './ledtoggle/ledtoggle.component';
import { ToggleComponent } from './toggle/toggle.component';
import { PatternComponent } from './pattern/pattern.component';

// ui-router states
const toggleState = { name: "toggle", component: ToggleComponent };
const patternState = { name: "pattern", component: PatternComponent };

@NgModule({
  declarations: [
    AppComponent,
    LEDToggleComponent,
    ToggleComponent,
    PatternComponent
  ],
  imports: [
    BrowserModule,
    UIRouterModule.forRoot({ 
      states: [toggleState, patternState], 
      useHash: true })
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
