package edu.umich.cse.eecs485;

import java.io.IOException;
import java.io.*;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.TextOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.TextInputFormat;
import org.apache.mahout.classifier.bayes.XmlInputFormat;
import java.util.HashSet;
import nu.xom.*;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.*;
import java.text.DecimalFormat;

public class InvertedIndex {

	// number of documnets
	static Long total_doc_num = new Long(0);
	// list of stop words
	private static HashSet<String> stop_words = new HashSet<String>();

	public static class ParseXMLMap extends Mapper<LongWritable, Text, Text, LongWritable> {
		public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {

			// docid
			Long doc_id = new Long(0);
			// document content
			String doc_body = "";
			// count the number of documents
			total_doc_num++;


			// Parse the xml and read data (page id and article body)
			// Using XOM library
			Builder builder = new Builder();
			try {
				// 1. get the document
				Document doc = builder.build(value.toString(), null);

				// 2. get docid
				Nodes nodeId = doc.query("//eecs485_article_id");
				doc_id = Long.parseLong(nodeId.get(0).getChild(0).getValue());

				// 3. get doc body
				Nodes nodeBody = doc.query("//eecs485_article_body");
				doc_body = nodeBody.get(0).getChild(0).getValue();
			}
			// indicates a well-formedness error
			catch (ParsingException ex) {
				System.out.println("Not well-formed.");
				System.out.println(ex.getMessage());
			} catch (IOException ex) {
				System.out.println("io exception");
			}

			/*

			 * loop through the words in the document
			 */
			Pattern pattern = Pattern.compile("\\w+");
			Matcher matcher = pattern.matcher(doc_body);
			// term frequency in this document: word, tf
			while (matcher.find()) {
				// Write the parsed token

				String word = matcher.group().toLowerCase();

				// consider stopWords
				if (!stop_words.contains(word)) {
					String new_key = word + " " + doc_id;
					context.write(new Text(new_key), new LongWritable(1));

				}
			}
		}
	}

	/*
	 * =========================================================================
	 * Reduce: (doc_id, [word) -> (word, array ( docid + tf))
	 * =========================================================================
	 */
	public static class ParseXMLReduce extends Reducer<Text, LongWritable, Text, Text> {
		public void reduce(Text key, Iterable<LongWritable> values, Context context)
				throws IOException, InterruptedException {
			String word_and_id = key.toString();
			StringTokenizer tokenizer = new StringTokenizer(word_and_id);

			String word = tokenizer.nextToken();
			Long doc_id = Long.parseLong(tokenizer.nextToken());
			Long tf = new Long(0);
			for (LongWritable value : values) {
				tf += 1;
			}

			String value = doc_id + " " + tf;
			context.write(new Text(word), new Text(value));
		}
	}

	/*
	 * =========================================================================
	 * MAP REDUCE 2
	 *
	 * =========================================================================
	 */
	/*
	 * =========================================================================
	 *
	 * Map: (word,(doc_id,tf)) => same
	 * =========================================================================
	 */
	public static class DocumentFrequencyMap extends Mapper<LongWritable, Text, Text, Text> {
		public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {
			/*
			 * (word, array ( docid + tf))
			 */
			StringTokenizer tokenizer = new StringTokenizer(value.toString());
			String new_key = tokenizer.nextToken();
			String new_value = tokenizer.nextToken();
			new_value += " " + tokenizer.nextToken();

			context.write(new Text(new_key), new Text(new_value));

		}
	}

	/*
	 * =========================================================================
	 *
	 *
	 * Reduce: (word,(doc_id,tf)) => (word, (doc_id,tf,df))
	 * =========================================================================
	 */
	public static class DocumentFrequencyReduce extends Reducer<Text, Text, Text, Text> {
		public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {

			List<String> cache = new ArrayList<String>();
			Long df = new Long(0);

			for (Text value : values) {
				df += 1;
				String text = new String();
				text = value.toString();
				cache.add(text);
			}
//			System.out.print(df);
			for (String value : cache) {
				String new_value = value + " " + df;
//				System.out.print(new_value);
				context.write(key, new Text(new_value));
			}


		}
	}

	public static class NormalizeMap extends Mapper<LongWritable, Text, LongWritable, Text> {
		public void map(LongWritable key, Text value, Context context)
				throws IOException, InterruptedException {

			String word;
			Long doc_id, tf, df;

			StringTokenizer tokenizer = new StringTokenizer(value.toString());

			word = tokenizer.nextToken();
			doc_id = Long.parseLong(tokenizer.nextToken());
			tf = Long.parseLong(tokenizer.nextToken());
			df = Long.parseLong(tokenizer.nextToken());

			String new_value = word + " " + tf + " " + df;
//			System.out.print(new_value+"\n");
			context.write(new LongWritable(doc_id), new Text(new_value));
		}
	}

	public static class NormalizeReduce extends Reducer<LongWritable, Text, LongWritable, Text> {
		public void reduce(LongWritable key, Iterable<Text> values, Context context)
				throws IOException, InterruptedException {

			Double normalizer = new Double(0);
			List<String> cache = new ArrayList<String>();

			for (Text value : values) {
				String text = value.toString();
				cache.add(text);
				StringTokenizer tokenizer = new StringTokenizer(text);
				String word;
				Double tf;
				Double df;

				word = tokenizer.nextToken();
				tf = Double.parseDouble(tokenizer.nextToken());
				df = Double.parseDouble(tokenizer.nextToken());
				Double tf_idf = tf * Math.log10(total_doc_num / df);
//				System.out.print("word: "+word+" tf: "+tf+" df: "+df+"\n");
				normalizer += tf_idf * tf_idf;
			}
//			System.out.print("normali	zer: "+normalizer+"\n");
			normalizer = Math.sqrt(normalizer);

			for (String value : cache) {
				StringTokenizer tokenizer = new StringTokenizer(value);
				String word;
				Long tf;
				Long df;

				word = tokenizer.nextToken();
				tf = Long.parseLong(tokenizer.nextToken());
				df = Long.parseLong(tokenizer.nextToken());
				Double tf_idf = tf * Math.log10(total_doc_num / new Double(df));
				String new_value = word + " " + tf_idf / normalizer + " " + df;
				context.write(key, new Text(new_value));
			}
		}
	}

	/*
	 * =========================================================================
	 * MAP REDUCE 4
	 *
	 * =========================================================================
	 */
	/*
	 * =========================================================================
	 * Map: (doc_id, (word, norm(tf*idf),df)) => word,(doc_id,norm(tf*idf),df)
	 * =========================================================================
	 */
	public static class FormattingMap extends Mapper<LongWritable, Text, Text, Text> {
		public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException {


			StringTokenizer tokenizer = new StringTokenizer(value.toString());
			Long doc_id = Long.parseLong(tokenizer.nextToken());
			String word = tokenizer.nextToken();
			Double norm = Double.parseDouble(tokenizer.nextToken());
			Long df = Long.parseLong(tokenizer.nextToken());

			String new_value = doc_id + " " + norm + " " + df;
			context.write(new Text(word), new Text(new_value));

		}
	}

	/*
	 * =========================================================================
	 * Reduce: (word,(doc_id,norm(tf*idf),df))=>same
	 * =========================================================================
	 */
	public static class FormattingReduce extends Reducer<Text, Text, Text, Text> {
		public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
			String result = new String();
			Long df = new Long(0);
			for (Text value : values){
				StringTokenizer tokenizer = new StringTokenizer(value.toString());
				Long doc_id = Long.parseLong(tokenizer.nextToken());
				Double norm = Double.parseDouble(tokenizer.nextToken());
				df = Long.parseLong(tokenizer.nextToken());
				result += doc_id + ":" + norm + " ";
			}
			result = df + " " + result;
			context.write(key, new Text(result));
		}
	}

	/*
	 * =========================================================================
	 * Main function: set up the three jobs Job1:
	 *
	 * =========================================================================
	 */
	public static void main(String[] args) throws Exception {

		// read stopWords
		BufferedReader bufferedReader = new BufferedReader(new FileReader("stop_words.txt"));
		String line = null;
		while ((line = bufferedReader.readLine()) != null) {
			line = line.toLowerCase();
			stop_words.add(line);
		}
		bufferedReader.close();
		/*** Job 1 ***/
		Configuration conf1 = new Configuration();

		conf1.set("xmlinput.start", "<eecs485_article>");
		conf1.set("xmlinput.end", "</eecs485_article>");

		Job job1 = new Job(conf1, "XmlParser");

		job1.setMapOutputKeyClass(Text.class);
		job1.setMapOutputValueClass(LongWritable.class);

		job1.setOutputKeyClass(Text.class);
		job1.setOutputValueClass(Text.class);

		job1.setMapperClass(ParseXMLMap.class);
		job1.setReducerClass(ParseXMLReduce.class);

		job1.setInputFormatClass(XmlInputFormat.class);
		job1.setOutputFormatClass(TextOutputFormat.class);

		Path job1Input = new Path(args[0]);
		Path job1Output = new Path("job1Output");
		FileInputFormat.addInputPath(job1, job1Input);
		FileOutputFormat.setOutputPath(job1, job1Output);

		job1.waitForCompletion(true);

		/*** Job 2 ***/
		Configuration conf2 = new Configuration();

		Job job2 = new Job(conf2, "DocumentFrequency");

		job2.setMapOutputKeyClass(Text.class);
		job2.setMapOutputValueClass(Text.class);

		job2.setOutputKeyClass(Text.class);
		job2.setOutputValueClass(Text.class);

		job2.setMapperClass(DocumentFrequencyMap.class);
		job2.setReducerClass(DocumentFrequencyReduce.class);

		job2.setInputFormatClass(TextInputFormat.class);
		job2.setOutputFormatClass(TextOutputFormat.class);

		Path job2Input = job1Output;
		Path job2Output = new Path("job2Output");
		FileInputFormat.addInputPath(job2, job2Input);
		FileOutputFormat.setOutputPath(job2, job2Output);

		job2.waitForCompletion(true);

		/*** Job 3 ***/
		Configuration conf3 = new Configuration();

		Job job3 = new Job(conf3, "Normalize");

		job3.setOutputKeyClass(LongWritable.class);
		job3.setOutputValueClass(Text.class);

		job3.setMapperClass(NormalizeMap.class);
		job3.setReducerClass(NormalizeReduce.class);

		job3.setInputFormatClass(TextInputFormat.class);
		job3.setOutputFormatClass(TextOutputFormat.class);

		Path job3Input = job2Output;
		Path job3Output = new Path("job3Output");
		FileInputFormat.addInputPath(job3, job3Input);
		FileOutputFormat.setOutputPath(job3, job3Output);

		job3.waitForCompletion(true);

		/*** Job 4 ***/
		Configuration conf4 = new Configuration();

		Job job4 = new Job(conf4, "Formatting");

		job4.setOutputKeyClass(Text.class);
		job4.setOutputValueClass(Text.class);

		job4.setMapperClass(FormattingMap.class);
		job4.setReducerClass(FormattingReduce.class);

		job4.setInputFormatClass(TextInputFormat.class);
		job4.setOutputFormatClass(TextOutputFormat.class);

		Path job4Input = job3Output;
		Path job4Output = new Path(args[1]);
		FileInputFormat.addInputPath(job4, job4Input);
		FileOutputFormat.setOutputPath(job4, job4Output);

		job4.waitForCompletion(true);
	}
}
