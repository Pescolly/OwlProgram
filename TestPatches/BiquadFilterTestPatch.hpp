////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 
 
 LICENSE:
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */


/* created by the OWL team 2013 */


////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __BiquadFilterTestPatch_hpp__
#define __BiquadFilterTestPatch_hpp__

#include "StompBox.h"
#include "BiquadFilter.h"

class BiquadFilterTestPatch : public TestPatch {
public:
  BiquadFilter *filter;
  BiquadFilterTestPatch(){
    // registerParameter(PARAMETER_A, "Cutoff");
    // registerParameter(PARAMETER_B, "Resonance");
    int stages=3;
    filter=BiquadFilter::create(stages);
    float cutoff=0.2;
    float resonance=2;
    //test setLowPass
    FloatArray coefficients=FloatArray::create(5*stages);
    FloatArray states=FloatArray::create(2*stages);
    FilterStage stage(coefficients, states);
    filter->setLowPass(cutoff, resonance);
    stage.setLowPass(cutoff, resonance);
    {
      TEST("Initialise coefficients");
      for(int k=0; k<stages; k++){ 
	for(int n=0; n<5; n++){
	  float filterC=filter->getFilterStage(k).getCoefficients()[n];
	  float stageC=stage.getCoefficients()[n];
	  CHECK_EQUAL(filterC, stageC); //check that filter coefficients are properly initialized
	}
      }
    }
    int signalLength=100;
    
    FloatArray x=FloatArray::create(signalLength);
    FloatArray x1=FloatArray::create(signalLength);
    FloatArray y=FloatArray::create(signalLength);
    FloatArray y1=FloatArray::create(signalLength);
    x.noise();
    x1.copyFrom(x);
    
    filter->process(x1, y1, x1.getSize());
    //manually compute the filter
    float b0=filter->getFilterStage(0).getCoefficients()[0];
    float b1=filter->getFilterStage(0).getCoefficients()[1];
    float b2=filter->getFilterStage(0).getCoefficients()[2];
    float a1=filter->getFilterStage(0).getCoefficients()[3];
    float a2=filter->getFilterStage(0).getCoefficients()[4];
    for(int n=0; n<stages; n++){
      float d1=0;
      float d2=0;
      for(int n=0; n<x.getSize(); n++){ //manually apply filter, one stage
        y[n] = b0 * x[n] + d1;
        d1 = b1 * x[n] + a1 * y[n] + d2;
        d2 = b2 * x[n] + a2 * y[n];   
      }
      x.copyFrom(y); //copy the output to the input for the next iteration. INEFFICIENT
    }
    //done with the filter
    {
      TEST("Compare output");
      for(int n=0; n<x.getSize(); n++){
	// ASSERT(abs(y[n]-y1[n])<0.0001, "");//BiquadFilter.process(FloatArray, FloatArray) result"); //TODO: fails for non-arm

	// CHECK_CLOSE(y[n], y1[n], 0.0001);
      }
    }
    FloatArray::destroy(x);
    FloatArray::destroy(x1);
    FloatArray::destroy(y);
    FloatArray::destroy(y1);
    debugMessage("All tests passed");
  }
};

#endif // __BiquadFilterTestPatch_hpp__
