#!/bin/bash

export JAVA_HOME=/usr

../bin/hadoop jar InvertedIndex.jar edu.umich.cse.eecs485.InvertedIndex ../dataset/test output
