import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';

import { AppComponent } from './app.component';
import { LEDToggleComponent } from './ledtoggle/ledtoggle.component';

@NgModule({
  declarations: [
    AppComponent,
    LEDToggleComponent
  ],
  imports: [
    BrowserModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
