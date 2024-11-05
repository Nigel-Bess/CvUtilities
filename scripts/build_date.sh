#!/bin/bash


BUILD=$(TZ='America/Los_Angeles' date +%s)
BUILD_STR=$(TZ='America/Los_Angeles' date)
FILE='../Fulfil.CPPUtils/include/Fulfil.CPPUtils/build.h'
GIT_BRANCH=$(git symbolic-ref --short HEAD)
GIT_SHA1=$(git describe  --always --abbrev=40 --dirty)
GIT_COMMIT_MSG=$(git log -1 --format=%s | sed 's/"/\\"/g')
GIT_DATE=$(git log -1 --format=%ad)

echo "#pragma once" > $FILE
echo "#include <string>
const std::string BUILD_DATE_STR = \""$BUILD_STR"\";
#define BUILD_DATE_SECONDS $BUILD" >> $FILE
echo "const std::string GIT_BRANCH = \""$GIT_BRANCH"\";
const std::string GIT_SHA1 = \""$GIT_SHA1"\";
const std::string GIT_DATE = \""$GIT_DATE"\";
const std::string GIT_COMMIT_MSG = \""$GIT_COMMIT_MSG"\";" >> $FILE
echo "inline std::string git_info(){
    return \""GIT branch: \'\"" + GIT_BRANCH + \""\' date: \"" + GIT_DATE + \""UTC \\n\\t\\t\\t   GIT SHA: [\"" + GIT_SHA1 + \""] \(\"" + GIT_COMMIT_MSG + \""\)\""; 
}" >> $FILE

