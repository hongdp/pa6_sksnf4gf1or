from flask import *
import requests
import json

from utils import *

search = Blueprint('search', __name__, template_folder='views')


@search.route(append_key('/search'), methods=['GET'])
def search_route():

    query = request.args.get('query')
    w = request.args.get('w')
    if w is None:
        w = 0.2
    if query is None or query == "":
       return render_template('search.html', w = w);


    # result_obj = requests.get('http://eecs485-09.eecs.umich.edu:6288/search?q=%s&w=%s' % (query, w))
    result_obj = requests.get('http://localhost:4000/search?q=%s&w=%s' % (query, w))
    result = json.loads(result_obj.text)
    page_info = []
    con = mysql.connection
    result_list = result['hits']
    for obj in result_list[:10]:

    #    page info : array of id,title.
        cur = con.cursor()
        cur.execute("SELECT article_title FROM Info WHERE article_id= %s " % (obj['id']))
        title = cur.fetchall()

        info = {}
        info['title'] = title[0][0]
        info['id'] =  obj['id']
        page_info.append(info)

    return render_template('search_result.html', query = query, pages = page_info, w = w)
