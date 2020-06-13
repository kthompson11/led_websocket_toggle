export class RangeConvertExponential {
  public a: number;
  public b: number;

  constructor(minRange1: number, maxRange1: number, minRange2: number, maxRange2: number) {
    this.b = Math.log(maxRange2/minRange2) / (maxRange1 - minRange1);
    this.a = minRange2 / Math.exp(this.b * minRange1);
  }

  /**
   * Applies an exponential conversion from range1 to range2.
   * @param x Number in range1 to convert to a number in range2.
   */
  expConvert(x: number): number {
    return this.a * Math.exp(this.b * x);
  }

  /**
   * Applies an inverse exponential conversion form range2 to range1.
   * @param x Number in range2 to convert to a number in range1.
   */
  expConvertInverse(x: number): number {
    return Math.log(x / this.a) / this.b;
  }
}
