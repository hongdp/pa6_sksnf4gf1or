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
    # con = mysql.connection
    result_list = result['hits']
    for obj in result_list:
    #
    #     cur = con.cursor()
    #     cur.execute("SELECT url FROM Photo WHERE picid= %s " % (obj['id']))
    #     url = cur.fetchall()
    #
    #     cur.execute("SELECT caption FROM Contain WHERE picid= %s " % (obj['id']))
    #     caption = cur.fetchall()
    #
        info = {}
    #     info['url'] = url[0][0]
    #     info['caption'] = caption[0][0]
    #     info['picid'] =  obj['id']
        page_info.append(info)

    return render_template('search_result.html', query = query, pages = page_info, w = w)
