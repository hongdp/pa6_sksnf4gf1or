import xml.etree.ElementTree as ET
import string
tree = ET.parse('../hadoop-example/dataset/mining.infobox.xml')
root = tree.getroot()

target = open('load_data.sql', 'a')
# count = 0;
for child in root:
    article_id = child[0].text
    article_title = child[1].text
    article_title = article_title.replace("'", "''")
    article_summary = child[2].text
    if not article_summary is None:
        # print(type(article_summary))
        article_summary = article_summary.replace("'", "''")
    line = "INSERT INTO Info(article_id, article_title, article_summary) VALUES('%s', '%s', '%s');" %(article_id, article_title, article_summary)

    target.write(line.encode('utf-8'))
    target.write('\n')

    # print('insert %s %s %s\n' %(article_id, article_title, article_summary))

# print(count)
target.close()
