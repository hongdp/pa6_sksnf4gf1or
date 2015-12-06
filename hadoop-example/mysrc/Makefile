all: wordcount invindex

wordcount:
	javac -cp .:../hadoop-core-1.0.1.jar:../lib/* -d bin WordCount.java 
	jar -cvf ./WordCount.jar -C bin/ .

invindex:
	javac -cp .:../hadoop-core-1.0.1.jar:../lib/* -d bin InvertedIndex.java XmlInputFormat.java
	jar -cvf ./InvertedIndex.jar -C bin/ .

xom:
	javac -cp .:../hadoop-core-1.0.1.jar:../lib/* -d bin XOMtest.java
	java -Xms1024m -Xmx1024m -cp .:../lib/*:bin/ edu.umich.cse.eecs485.XOMtest

clean:
	rm -rf *.jar bin/* output job1Output job2Output job3Output
