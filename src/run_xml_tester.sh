#!/bin/bash

export XERCES_DIR=/opt/tools/local/Xerces/C++/64bit/3.1.1/lib
export LD_LIBRARY_PATH=${XERCES_DIR}:${LD_LIBRARY_PATH}

## ../bin/Linux.GCC64/xml_tester child_sibling.xml
## ../bin/Linux.GCC64D/xml_tester data_query.xml
../bin/Linux.GCC64D/xml_tester data_req_test.xml
