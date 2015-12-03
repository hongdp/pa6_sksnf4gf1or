#!/bin/bash
export JAVA_HOME=/usr
../bin/hadoop jar WordCount.jar edu.umich.cse.eecs485.WordCount ../dataset/test output
