from utils import *
from flask import *
# -*- coding: utf-8 -*-

main = Blueprint('main', __name__, template_folder='views')


@main.route(append_key('/'))
def main_route():
    con = mysql.connection
    cur = con.cursor()
    if session_is_valid(session):
        renew_session(session)
        username = session['username']
        sql_get_all_albums = "SELECT DISTINCT Album.albumid, Album.title, Album.username FROM Album, AlbumAccess WHERE Album.access='public' OR Album.username='%s' OR Album.albumid=AlbumAccess.albumid AND AlbumAccess.username='%s' ORDER BY Album.username" % (
            username, username)
        # sql_get_all_albums = "SELECT albumid, title, username FROM (SELECT albumid, title, \
        # username FROM Album WHERE access='public' OR username='%s' UNION SELECT \
        # Album.albumid as albumid, title, Album.username FROM AlbumAccess, Album \
        # WHERE AlbumAccess.albumid = Album.albumid AND AlbumAccess.username='%s') \
        # as t1 ORDER BY username"%(session['username'], session['username'])
        cur.execute(sql_get_all_albums)
        albums = cur.fetchall()

        sql_get_count = "SELECT Albums.username, count(*) from (%s) AS Albums \
        GROUP BY Albums.username" % sql_get_all_albums
        cur.execute(sql_get_count)
        albums_count = cur.fetchall()
        rowspans = {}
        for album_count in albums_count:
            rowspans[album_count[0]] = album_count[1]
        # print session['username']
        return render_template("index.html", username=session['username'], login=True, albums=albums, rowspans=rowspans)
    elif session_is_expired(session):
        session.clear()
    cur.execute("SELECT albumid, title FROM Album WHERE access='public' ORDER BY username")
    albums = cur.fetchall()
    return render_template("index.html", login=False, albums=albums)

#
# @app.route('/logout')
# def logout():
#     # remove the username from the session if it's there
#     session.pop('username', None)
#     return redirect(url_for('index'))
