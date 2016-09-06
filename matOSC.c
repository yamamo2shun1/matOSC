/*
 * Copylight (C) 2016, Shunichi Yamamoto, tkrworks.net
 *
 * This file is part of matOSC.
 *
 * matOSC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option ) any later version.
 *
 * matOSC is distributed in the hope that it will be useful,
 * but WITHIOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with matOSC. if not, see <http:/www.gnu.org/licenses/>.
 *
 * macOSC.c,v.0.1.0 2016/09/06
 */

#include "mex.h"

#include <string.h>

//#define DEBUG
#define DEBUG_PRINT_PREFIX ":::DEBUG::"

#define MAX_BUF_SIZE    96//64
#define MAX_PACKET_SIZE 256//192// 1024
#define MAX_MESSAGE_LEN 256// 160
#define MAX_ADDRESS_LEN 64
#define MAX_ARGS_LEN    48// 40

mwSize oscTotalSize;
unsigned char sndOSCData[MAX_MESSAGE_LEN];

void addOSCIntArgument(int value);
void addOSCFloatArgument(float value);
void addOSCStringArgument(char* str);
void clearOSCMessage(void);

void setOSCAddress(char* prefix, char* command)
{
  mwSize prefixSize = (mwSize)(strchr(prefix, 0) - prefix);
  mwSize commandSize = (mwSize)(strchr(command, 0) - command);
  mwSize addressSize = prefixSize + commandSize;
  mwSize zeroSize = 0;
    
  memset(sndOSCData, 0, MAX_MESSAGE_LEN);
  oscTotalSize = 0;
    
  sprintf(sndOSCData, "%s%s", prefix, command);
    
  zeroSize = (addressSize ^ ((addressSize >> 3) << 3)) == 0 ? 0 : 8 - (addressSize ^ ((addressSize >> 3) << 3));
  if(zeroSize == 0)
    zeroSize = 4;
  else if(zeroSize > 4 && zeroSize < 8)
    zeroSize -= 4;

  oscTotalSize = (addressSize + zeroSize);
}

void setOSCTypeTag(char* type)
{
  mwSize typeSize = (mwSize)(strchr(type, 0) - type);
  mwSize zeroSize = 0;

  sprintf((sndOSCData + oscTotalSize), ",%s", type);

  typeSize++;
  zeroSize = (typeSize ^ ((typeSize >> 2) << 2)) == 0 ? 0 : 4 - (typeSize ^ ((typeSize >> 2) << 2));
  if(zeroSize == 0)
    zeroSize = 4;
  
  oscTotalSize += (typeSize + zeroSize);
}

void addOSCIntArgument(int value)
{
  sndOSCData[oscTotalSize++] = (value >> 24) & 0xFF;
  sndOSCData[oscTotalSize++] = (value >> 16) & 0xFF;
  sndOSCData[oscTotalSize++] = (value >> 8) & 0xFF;
  sndOSCData[oscTotalSize++] = (value >> 0) & 0xFF;
}

void addOSCFloatArgument(float value)
{
  unsigned char* fchar = NULL;
  fchar = (unsigned char *)&value;
  sndOSCData[oscTotalSize++] = fchar[3] & 0xFF;
  sndOSCData[oscTotalSize++] = fchar[2] & 0xFF;
  sndOSCData[oscTotalSize++] = fchar[1] & 0xFF;
  sndOSCData[oscTotalSize++] = fchar[0] & 0xFF;
}

void addOSCStringArgument(char* str)
{
  mwSize cstr_len = 0;
  while(*str)
  {
    sndOSCData[oscTotalSize + cstr_len] = *(str++) & 0xFF;
    cstr_len++;
  }
  oscTotalSize += ((cstr_len / 4) + 1) * 4;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mwSize i = 0;
    
  char* prefix_str;
  char* command_str;
  char* typetag_str;
    
  mwSize typetag_len = 0;
  mwSize dims[] = {1, 1};
  mxChar* output_ptr;
    
  prefix_str = mxArrayToString(prhs[0]);
  command_str = mxArrayToString(prhs[1]);
  typetag_str = mxArrayToString(prhs[2]);
    
  setOSCAddress(prefix_str, command_str);
  setOSCTypeTag(typetag_str);
  typetag_len = (mwSize)strlen(typetag_str);

#if defined(DEBUG)    
  mexPrintf("%s typetag_len = %d", DEBUG_PRINT_PREFIX, typetag_len);
#endif
    
  for(i = 0; i < typetag_len; i++)
  {
    switch(typetag_str[i])
    {
      case 'f':
        addOSCFloatArgument((float)mxGetScalar(prhs[3 + i]));
        break;
      case 'i':
        addOSCIntArgument((int)mxGetScalar(prhs[3 + i]));
        break;
      case 's':
        addOSCStringArgument(mxArrayToString(prhs[3 + i]));
        break;
    }
  }
    
  dims[0] = oscTotalSize;

#if defined(DEBUG)    
  mexPrintf("%s oscTotalSize = %d\n", DEBUG_PRINT_PREFIX, oscTotalSize);
    
  for(i = 0; i < oscTotalSize; i++)
  {
    if(sndOSCData[i] == 0x00)
      mexPrintf("%02X ", sndOSCData[i]);
    else
      mexPrintf("%d:%02X(%c) ", sndOSCData[i], sndOSCData[i], sndOSCData[i]);
  }
  mexPrintf("\n");
#endif

  plhs[0] = mxCreateNumericArray(2, dims, mxUINT8_CLASS, mxREAL);
    
  output_ptr = mxGetData(plhs[0]);
  memcpy(output_ptr, sndOSCData, oscTotalSize);

  mxFree(prefix_str);
  mxFree(command_str);
  mxFree(typetag_str);
    
  return;
}
