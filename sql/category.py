import xml.etree.ElementTree as ET
tree = ET.parse('../hadoop-example/dataset/mining.category.xml')
root = tree.getroot()

target = open('load_data.sql', 'a')

for child in root:
    article_id = child[0].text
    article_category = child[2].text
    article_category = article_category.replace("'", "''")
    line = "INSERT INTO Category(article_id, article_category) VALUES('%s', '%s');" %(article_id, article_category)
    target.write(line.encode('utf-8'))
    target.write('\n')
    # print('insert %s %s\n' %(article_id, article_category))


target.close()
