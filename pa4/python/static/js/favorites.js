function Favorites(numElement, userElement, favoriteButton, picid, username) {
  this.numElement = numElement;
  this.userElement = userElement;
  this.favoriteButton = favoriteButton;
  this.picid = picid;
  this.username = username;
  if (this.favoriteButton){
    favoriteButton.addEventListener("click", this, false);
  }
}


Favorites.prototype.handleEvent = function(e) {
  if (e.type === "click") {
    this.update();
  }
}

Favorites.prototype.change = function(ret) {
  this.numElement.innerHTML = ret['num_favorites'];
  this.userElement.innerHTML = ret['latest_favorite'];
}

Favorites.prototype.update = function() {
  var local_pic_id = this.picid;
  var change_function = this.change.bind(this);
  makeFavoritesPostRequest(this.picid, this.username, function() {
    console.log('POST successful.');
    makeFavoritesRequest(local_pic_id, change_function);
  });
}

function makeFavoritesPostRequest(picid, username, cb) {
  var data = {
    'id': picid,
    'username': username
  };

  qwest.post('/sksnf4gf1or/pa3/pic/favorites', data, {
    dataType: 'json',
    responseType: 'json'
  }).then(function(xhr, resp) {
    cb(resp);
  });
}

function makeFavoritesRequest(picid, cb) {
  qwest.get('/sksnf4gf1or/pa3/pic/favorites?id=' + picid)
    .then(function(xhr, resp) {
      cb(resp);
    });
}

function initFavorites(picid, username) {
  var num_favorites = document.getElementById("num-favorites");
  var user_favorites = document.getElementById("user-favorites");
  var favorite_button = document.getElementById("favorite-button");
  var favoritesBinding = new Favorites(num_favorites, user_favorites, favorite_button, picid, username);

  makeFavoritesRequest(picid, function(resp) {
    favoritesBinding.change(resp);
  });

  setInterval(function() {
   makeFavoritesRequest(picid, function(resp) {
      favoritesBinding.change(resp);
    });
}, 10000);
}
