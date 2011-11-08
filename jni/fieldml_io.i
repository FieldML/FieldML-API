%module FieldmlIo
%include enums.swg
%include arrays_java.i

/*
  Currently, all int* and double* arguments are arrays, not single references.
 */
%apply int[] {int *}
%apply double[] {double *}
%apply short {int16_t}
%apply int {int32_t}
%apply long {int64_t}

%javaconst(1);

/*
  For buffers that are meant to be filled in the C code, we need to pass in a StringBuilder, not a
  String. Assume any declaration of 'char *buffer' is such a thing.
  
  NOTE: Const-correctness in the original header file should allow all 'const char *' to be mapped
  to a String, and all 'char *' to be mapped to a StringBuilder. 
 */

%typemap(jni) char *buffer "jobject"
%typemap(jtype) char *buffer "StringBuilder"
%typemap(jstype) char *buffer "StringBuilder"

%typemap(in) char *buffer
{
  $1 = NULL;
  if($input != NULL)
  {
    jmethodID setLengthID;
    jclass bufferClass = jenv->GetObjectClass($input);
    jmethodID toStringID = jenv->GetMethodID(bufferClass, "toString", "()Ljava/lang/String;");
    jstring js = (jstring) jenv->CallObjectMethod($input, toStringID);

    const char *pCharStr = jenv->GetStringUTFChars(js, 0);

    jmethodID capacityID = jenv->GetMethodID(bufferClass, "capacity", "()I");
    jint capacity = jenv->CallIntMethod($input, capacityID);
    $1 = (char *) malloc(capacity+1);
    strcpy($1, pCharStr);

    jenv->ReleaseStringUTFChars( js, pCharStr);

    setLengthID = jenv->GetMethodID(bufferClass, "setLength", "(I)V");
    jenv->CallVoidMethod($input, setLengthID, (jint) 0);
  }
}

%typemap(argout) char *buffer
{
  if($1 != NULL)
  {
    jstring newString = jenv->NewStringUTF($1);
    jclass bufferClass = jenv->GetObjectClass($input);
    jmethodID appendStringID = jenv->GetMethodID(bufferClass, "append", "(Ljava/lang/String;)Ljava/lang/StringBuilder;");
    jenv->CallObjectMethod($input, appendStringID, newString);

    free($1);
    $1 = NULL;
  }  
}

%typemap(freearg) char *buffer ""

%typemap(javain) char *buffer "$javainput"


/*
  Need to create a fixed-length array for Java. Get the length from the API.
 */
%typemap(out) const int* Fieldml_GetSwizzleData
%{$result = SWIG_JavaArrayOutInt(jenv, $1, Fieldml_GetSwizzleCount( arg1, arg2 ) ); %}


/*
  Tell the Fieldml JNI class to load its own library.
 */
%pragma(java) jniclasscode=%{
    static
    {
        System.loadLibrary("fieldml_jni_0.3");
    }
%}


%{
typedef int int32_t;
#include "FieldmlIoApi.h"
%}

%include "FieldmlIoApi.h"

