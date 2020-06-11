import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { LEDToggleComponent } from './ledtoggle.component';

describe('LEDToggleComponent', () => {
  let component: LEDToggleComponent;
  let fixture: ComponentFixture<LEDToggleComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ LEDToggleComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LEDToggleComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
