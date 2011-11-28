#!/bin/bash
cd jni
swig -I/home/andrew/Documents/fieldml.api/core/src -package fieldml.jni -outdir ./fieldml/jni -c++ -java fieldml_api.i
javac ./fieldml/jni/*.java
jar -cf fieldml.jar ./fieldml/jni/*.class
cd ..
g++ -ggdb -O0 -shared -o libfieldml.so ./core/src/*.cpp -I/usr/lib/jvm/java-6-openjdk-amd64/include/ -Icore/src/ `pkg-config --cflags libxml-2.0` `pkg-config --libs libxml-2.0` jni/*.cxx -fPIC
if [[ ! -e libfieldml_jni_0.3.so ]]; then
  ln -s `pwd`/libfieldml.so libfieldml_jni_0.3.so
fi
