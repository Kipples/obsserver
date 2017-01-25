var app = angular.module('test', ['dndLists']);

app.controller('LayoutCtrl', function($scope, $http, $timeout) {
    $scope.user_list_selected = null;
    $scope.layout = null;
    $scope.scenes = null;

    $scope.audio = null;

    $scope.api_key = '';
    $scope.api_key_input = '';

    $scope.setAPIKey = function(key) {
	$scope.api_key = key;
	getLayout();
	getScenes();
	getAudioSource();
    }

    var layout_hash = null;
    var scenes_hash = null;

    var scenes_ignore_filter = ['main', ''];

    $scope.useFilter = true;

    var updateLayoutHash = function() {
	layout_hash = JSON.stringify($scope.layout);
    }

    var updateSceneHash = function() {
	scenes_hash = JSON.stringify($scope.scenes);
    }

    $scope.hasChanges = function () {
	return JSON.stringify($scope.layout) !== layout_hash || JSON.stringify($scope.scenes) !== scenes_hash;
    };

    var getLayout = function() {
	$http({method: 'GET',
	       url: '/api/layout',
	       headers : {
		   'Client-ID': $scope.api_key
	       }
	      }).then(function(res) {
		  $scope.layout = res.data;
		  updateLayoutHash();
	      }).catch(function(res) {
		  console.log("error getting layout");
	      });
    };

    var getScenes = function() {
	$http({method: 'GET',
	       url: '/api/scenes',
	       headers : {
		   'Client-ID': $scope.api_key
	       }
	      }).then(function(res) {
		  $scope.scenes = res.data;
		  updateSceneHash();
	      }).catch(function(res) {
		  console.log("error getting scenes");
	      });
    };

    var getAudioSource = function () {
	$http({method: 'GET',
	       url: '/api/sources/audio',
	       headers : {
		   'Client-ID': $scope.api_key
	       }
	      }).then(function(res) {
		  $scope.audio = res.data;
	      }).catch(function(res) {
		  console.log("error getting audio source");
	      });
    };
    
    $scope.drop = function(list, item, newIndex) {
	var oldIndex = list.indexOf(item);
	// handle same-list (reorder)
	if (oldIndex !== -1) {
	    if (newIndex > oldIndex) --newIndex;
	    list.splice(oldIndex, 1);
	    list.splice(newIndex, 0, item);
	    return false; // cancel drop (dnd-moved will not executed)
	}
	// otherwise, proceed by default
	return item;
    };

    $scope.undoAllChanges = function() {
	getLayout();
    };

    $scope.postLayout = function () {
	$http({method: 'POST',
	       url: '/api/layout',
	       headers: {
		   'Client-ID': $scope.api_key,
		   'Content-Type': 'application/json'
	       },
	       data: JSON.stringify($scope.layout)
	      }).then(function(res) {
		  $scope.layout = res.data;
		  updateLayoutHash();
		  $http({method: 'PUT',
			 url: '/api/layout/reflow',
			 headers: {
			     'Client-ID': $scope.api_key
			 }
			}).then(function (res) {
			    $scope.layout = res.data;
			    updateLayoutHash();
			}).catch(function (res) {
			    console.log("error making reflow request");
			})
	      }).catch(function(res) {
		  console.log("error posting layout");
	      })
    };

    $scope.toggleMute = function(item) {
	$scope.layout[item].muted = !$scope.layout[item].muted;
    }
    
    $scope.isMuted = function(item) {
	return $scope.layout[item].muted;
    }

    $scope.changeScene = function(scene) {
	var endpoint = '/api/scenes/' + scene + '/active';
	$http({method: 'PUT',
	       url: endpoint,
	       headers: {
		   'Client-ID': $scope.api_key
	       }
	      }).then(function (res) {
		  getScenes();
	      }).catch(function (res) {
		  console.log("error changing scenes");
	      });
    };

    $scope.setDelay = function() {
	$http({method: 'POST',
	       url: '/api/sources/audio',
	       headers: {
		   'Client-ID': $scope.api_key,
		   'Content-Type': 'application/json'
	       },
	       data: JSON.stringify($scope.audio)
	      }).then(function(res) {
		  $scope.audio = res.data
	      }).catch(function(res) {
		  console.log("error setting delay");
	      });
    };

    $scope.filterScenes = function(scene) {
	var scene_num = Number(scene.slice(-1));
	var inUse = scene_num < ($scope.layout.active_user_list.length / $scope.layout.num_streams);
	if($scope.useFilter)
	    return inUse && (scene.startsWith($scope.layout.num_streams) || (scenes_ignore_filter.indexOf(scene) > -1));
	return true;
    };

    $scope.startStream = function() {
	$http({method: 'PUT',
	       url: '/api/outputs/live_stream/active',
	       headers: {
		   'Client-ID': $scope.api_key
	       }
	      }).then(function (res) {
		  
	      }).catch(function (res) {
		  console.log("error starting stream");
	      });
    };

    $scope.stopStream = function() {
	$http({method: 'DELETE',
	       url: '/api/outputs/live_stream/active',
	       headers: {
		   'Client-ID': $scope.api_key
	       }
	      }).then(function (res) {
		  
	      }).catch(function (res) {
		  console.log("error stopping stream");
	      });
    };

    $scope.unmutedCount = function () {
	var count = 0;
	for(var i = 0; i < $scope.layout.active_user_list.length; i++) {
	    if(!($scope.layout[$scope.layout.active_user_list[i]].muted))
		count++;
	}
	return count;
    }
});
