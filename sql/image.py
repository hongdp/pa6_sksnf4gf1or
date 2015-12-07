import xml.etree.ElementTree as ET
tree = ET.parse('../hadoop-example/dataset/mining.imageUrls.xml')
root = tree.getroot()

target = open('load_data.sql', 'a')

for child in root:
    article_id = child[0].text
    png_url = ''
    if len(child[2]):
        png_url = child[2][0].text
    line = "UPDATE Info SET png_url = '%s' WHERE article_id = '%s';" %(png_url, article_id)
    target.write(line.encode('utf-8'))
    target.write('\n')
    # print('update %s %s\n' %(article.eecs485_article_id, article.eecs485_article_category))


target.close()
