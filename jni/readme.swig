Java binding should be generated with the following command-line (or similar):

swig -I$(FIELDML_PATH) -package fieldml.jni -outdir ./fieldml/jni -c++ -java fieldml_api.i
