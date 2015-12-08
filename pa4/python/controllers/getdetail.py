from flask import *
import json

from utils import *

getdetail = Blueprint('getdetail', __name__, template_folder='views')


@getdetail.route(append_key('/getdetail'), methods=['GET'])
def detail_route():
    req_json = request.get_json()

    pageid = req_json.get('page_id')

    if pageid is None:
        response = json.jsonify(error='You did not provide an id parameter.', status=404)
        response.status_code = 404
        return response

    query = "SELECT article_summary, png_url FROM Info WHERE article_id='%s';" % pageid
    con = mysql.connection
    cur = con.cursor()
    cur.execute(query)
    info = cur.fetchall()
    if len(info) == 0:
        response = json.jsonify(error='Invalid id. The id does not exist.', status=422)
        response.status_code = 422
        return response

    query = "SELECT article_category FROM Category WHERE article_id='%s';" % pageid
    cur.execute(query)
    result = cur.fetchall()
    categories = []
    for category in result:
        categories.append(category[0])

    response = json.jsonify(imageUrl=info[0][2], summary=info[0][1], categories=categories, status=201)
    response.status_code = 201
    return response
