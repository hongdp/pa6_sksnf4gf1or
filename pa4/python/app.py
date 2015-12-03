from flask import Flask, render_template
import controllers
from utils import mysql, config_app, mail
app = Flask(__name__, template_folder='views', static_url_path='/static')


app.register_blueprint(controllers.main)
app.register_blueprint(controllers.album)
app.register_blueprint(controllers.albums)
app.register_blueprint(controllers.pic)
app.register_blueprint(controllers.login)
app.register_blueprint(controllers.user)
app.register_blueprint(controllers.useredit)
app.secret_key = '\xfc\x9c\x90\xf0\r9\x88\xf6D\xc1\n\x96]\x7fzO8\x88O?\xa7b\xb9\xe1'


@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404


config_app(app)
mysql.init_app(app)
mail.init_app(app)

# comment this out using a WSGI like gunicorn
# if you dont, gunicorn will ignore it anyway
if __name__ == '__main__':
    # listen on external IPs
    app.run(host='0.0.0.0', port=3000, debug=True)
