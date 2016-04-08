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


Contains code by Harrison Ainsworth / HXA7241 : 2004-2011.
from http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.
* The name of the author may not be used to endorse or promote products derived
  from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.


Contains code from http://www.icsi.berkeley.edu/pubs/techreports/TR-07-002.pdf
Vinyals, O., G. Friedland, and N. Mirghafori. "Revisiting a basic function on current CPUs: a fast logarithm implementation with adjustable accuracy." International Computer Science Institute (2007).

 */

#ifndef FASTTRANS_HPP
#define FASTTRANS_HPP

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "FloatArray.h"

/**
 * This class computes logarithm with a fast 
 * algorithm described in
 * http://www.icsi.berkeley.edu/pubs/techreports/TR-07-002.pdf
 * It internally allocates a table of size 2^precision.
 *
 * If you already instantiated an object of the class FastPow,
 * then you can use the FastLog object already instantiated in there.
*/

class FastLog{
private:
  /*
   * lookup_table is a pointer to a floating point array
   *(memory has to be allocated by the user) of 2^n positions.
   */
  float* lookup_table;
  unsigned int _tableLength;
  unsigned int n;
  float _ilog2;
public:
  FastLog(){
    lookup_table = NULL;
  };
  ~FastLog(){
    free(lookup_table);
  }
  /**
   * @param precision the number of bits to be taken from the mantissa.
   *   it must be (0<=precision<=23)
   */
  void setup(unsigned int precision){
    n = precision;
    
    lookup_table = (float*)malloc(sizeof(float)*(1 << n)) ;
    fill_icsi_log_table();
  }
  
  /* Creates the ICSILog lookup table. Must be called
  once before any call to icsi_log().
  */
  void fill_icsi_log_table()
  {
    float numlog;
    int *const exp_ptr = ((int*)&numlog);
    int x = *exp_ptr; //x is the float treated as an integer
    x = 0x3F800000; //set the exponent to 0 so numlog=1.0
    *exp_ptr = x;
    int incr = 1 << (23-n); //amount to increase the mantissa
    int p=pow(2,n);
    for(int i=0;i<p;++i)
    {
      lookup_table[i] = log2(numlog); //save the log value
      x += incr;
      *exp_ptr = x; //update the float value
    }
  }
  
  /* Computes an approximation of log(val) quickly.
  val is a IEEE 754 float value, must be >0.
  returns: log(val). No input checking performed.
  
  @param arg the argument of the logarithm. 
  @return log(arg), or 0 if arg<=0
  */
  float log(float arg)
  {
    if(arg <= 0) 
      return 0;
    int *const exp_ptr = ((int*)&arg);
    int x = *exp_ptr; //x is treated as integer
    const int log_2 = ((x >> 23) & 255) - 127;//exponent
    x &= 0x7FFFFF; //mantissa
    x = x >> (23-n); //quantize mantissa
    arg = lookup_table[x]; //lookup precomputed arg
    arg = ((arg + log_2)* 0.69314718);
    return arg; //natural logarithm
  }
  
 /** 
   * Computes the logarithm.
   * 
   * Computes an approximation of log_{base}(arg)
   * using the internal FastLog object
   *
   * @param base the base of the logarithm. 
   * @param arg the argument of the logaritm
   * @return log(arg), or 0 if arg<=0
   */
  float log(float base, float arg){
    //log_b(x) = log_e(x) / log_e(b)
    return this->log(arg) / this->log(base);
  }

};


/**
 * This class computes pow with a fast algorithm described in 
 * http://www.hxa.name/articles/content/fast-pow-adjustable_hxa7241_2007.html
 * It internally allocates a table of size 2^precision and a 
 * FastLog object of the same precision.
 * 
*/

class FastPow {
private:
  /**
   * @_ilog2      one over log, to required radix, of two
   * @_pTable     length must be 2 ^ _precision
  */

  unsigned int* _pTable;
  unsigned int _tableLength;
  static constexpr float _2p23 = 8388608.0f;
  unsigned int _precision;
  float _ilog2;
  FastLog fastLog;
public:
  FastPow(){
    _pTable = NULL;
  }
  ~FastPow(){
    free(_pTable);
  }
  
  /**
   * Initialize powFast lookup table.
   *
   * @param precision number of mantissa bits used, must be 0 =< precision <= 18
   */
  void setup(const unsigned int precision)
  {
    free(_pTable);
    _precision = precision;
    fastLog.setup(_precision);
    _tableLength = 1 << _precision;
    _pTable=(unsigned int*)malloc(sizeof(unsigned int)*_tableLength);
    if(_pTable == NULL){
      exit(1);
    }
    /* step along table elements and x-axis positions */
    float zeroToOne = 1.0f / ((float)(1 << _precision) * 2.0f);        /* A */
    int   i;                                                          /* B */
    for( i = 0;  i < (1 << _precision);  ++i )                         /* C */
    {
      /* make y-axis value for table element */
      const float f = ((float)::pow( 2.0f, zeroToOne ) - 1.0f) * _2p23; //make sure your call ::pow, otherwise you will be calling this class' method!
      _pTable[i] = (unsigned int)( f < _2p23 ? f : (_2p23 - 1.0f) );
      zeroToOne += 1.0f / (float)(1 << _precision);
     }                                                                 /* D */
  }

  /**
   * Get pow (fast!).
   *
   * @param value power to raise radix to
   *
   * @return radix^exponent
   */
  float getPow(float exponent)
  {
    /* strip the sign */
    static const float _2p23 = 8388608.0f;
    /* build float bits */
    const int i = (int)( (exponent * (_2p23 * _ilog2)) + (127.0f * _2p23) );

    /* replace mantissa with lookup */
    const int it = (i & 0xFF800000) | _pTable[(i & 0x7FFFFF) >> (23 - _precision)];

    /* convert bits to float */
    float out = *(const float*)( &it );
    
    return out;
  }
  
  /**
   * Sets the radix.
   *
   * @param base radix
   */
  void setBase(const float base){
    _ilog2 = computeIlog(base);
  }
  
  /**
   * Compute fast pow.
   *
   * @param base radix
   * @param exponent exponent
   *
   * @return radix^exponent
   */
  float pow(float base, float exponent){
    setBase(base);
    return getPow(exponent);
  }
  
  /**
   * Compute fast pow.
   *
   * @param ilog2 log(base)*1.44269504088896
   * @param exponent exponent
   *
   * @return radix^exponent
   */
  float powIlog(float ilog2, float exponent){
    _ilog2 = ilog2;
    return getPow(exponent);
  }

  /** 
   * Computes the logarithm.
   * 
   * Computes an approximation of logn(val)
   * using the internal FastLog object
   *
   * @param arg the argument of the logarithm. 
   * @return log(arg), or 0 if arg<=0
   */
  float log(float arg){
    return fastLog.log(arg);
  }
  
   /** 
   * Computes the logarithm.
   * 
   * Computes an approximation of log_{base}(arg)
   * using the internal FastLog object
   *
   * @param base the base of the logarithm. 
   * @param arg the argument of the logaritm
   * @return log(arg), or 0 if arg<=0
   */
  float log(float base, float arg){
    return fastLog.log(base, arg);
  }

  /**
   * Computes the ilog2 to pass to powIlog
   *
   * @param base the base of the logarithm
   * 
   * @return the ilog2
   */
  float const computeIlog(float base){
    return log(base)*1.44269504088896;
  }
};

#endif /* FASTTRANS_HPP */
