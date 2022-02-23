function getInfoMessage() {
  var xhttp = new XMLHttpRequest();
  xhttp.onload = function() {
    if (this.status == 200) {
      document.getElementById("InfoMessage").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "info", true);
  xhttp.send();
}

function getJsonData(x) {
  var xhttp = new XMLHttpRequest();
  xhttp.onload = function() {
    if (this.status == 200) {
	 var jsonResponse = JSON.parse(this.responseText);
      if(jsonResponse.innerHTML){   
          var innerHtmlKeys = Object.keys(jsonResponse.innerHTML);
		  innerHtmlKeys.forEach((key, index) => {
			var element = document.getElementById(key);
			if (element) {
				element.innerHTML = jsonResponse.innerHTML[key].value;
			}
		  });
      }
      if(jsonResponse.innerValues){   
          var innerValueKeys = Object.keys(jsonResponse.innerValues);
		  innerValueKeys.forEach((key, index) => {
			var element = document.getElementById(key);
			if (element) {
				element.value = jsonResponse.innerValues[key].value;
			}
		  });
      }
      if(jsonResponse.innerSrc){          
          var innerSrckeys = Object.keys(jsonResponse.innerSrc);
		  innerSrckeys.forEach((key, index) => {
			var element = document.getElementById(key);
			if (element) {
				element.src = jsonResponse.innerSrc[key].value;
			}
		  });
      }
      if(jsonResponse.innerTitles){     
          var innerTitlekeys = Object.keys(jsonResponse.innerTitles);
		  innerTitlekeys.forEach((key, index) => {
			var element = document.getElementById(key);
			if (element) {
				element.title = jsonResponse.innerTitles[key].value;
			}
		  });
      }
      if(jsonResponse.innerChecks){     
          var innerCheckkeys = Object.keys(jsonResponse.innerChecks);
		  innerCheckkeys.forEach((key, index) => {
			var element = document.getElementById(key);
			if (element && jsonResponse.innerChecks[key]['value'] == true) {
				element.checked = true;
			}
		  });
      }
    }
  };
  xhttp.open("GET", x, true);
  xhttp.send();
}

function getStaticData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onload = function() {
    if (this.status == 200) {
	 var jsonResponse = JSON.parse(this.responseText);
      if(jsonResponse.innerHTML){   
          var innerHtmlKeys = Object.keys(jsonResponse.innerHTML);
		  innerHtmlKeys.forEach((key, index) => {
			var element = document.getElementById(key);
			if (element) {
				element.innerHTML = jsonResponse.innerHTML[key].value;
			}
		  });
      }
      if(jsonResponse.innerValues){   
          var innerValueKeys = Object.keys(jsonResponse.innerValues);
		  innerValueKeys.forEach((key, index) => {
			var element = document.getElementById(key);
			if (element) {
				element.value = jsonResponse.innerValues[key].value;
			}
		  });
      }
      if(jsonResponse.innerSrc){          
          var innerSrckeys = Object.keys(jsonResponse.innerSrc);
		  innerSrckeys.forEach((key, index) => {
			var element = document.getElementById(key);
			if (element) {
				element.src = jsonResponse.innerSrc[key].value;
			}
		  });
      }
      if(jsonResponse.innerTitles){     
          var innerTitlekeys = Object.keys(jsonResponse.innerTitles);
		  innerTitlekeys.forEach((key, index) => {
			var element = document.getElementById(key);
			if (element) {
				element.title = jsonResponse.innerTitles[key].value;
			}
		  });
      }
      if(jsonResponse.innerChecks){     
          var innerCheckkeys = Object.keys(jsonResponse.innerChecks);
		  innerCheckkeys.forEach((key, index) => {
			var element = document.getElementById(key);
			if (element && jsonResponse.innerChecks[key]['value'] == true) {
				element.checked = true;
			}
		  });
      }
    }
  };
  xhttp.open("GET", "indexStatic", true);
  xhttp.send();
}

function getSaveState() {
  var xhttp = new XMLHttpRequest();
  xhttp.onload = function() {
    if (this.status == 200) {
      document.getElementById("SaveMessage").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "settingssaved", true);
  xhttp.send();
}
function mouseoverPass(obj) {
  var obj = document.getElementById('myPassword');
  obj.type = "text";
}
function mouseoutPass(obj) {
  var obj = document.getElementById('myPassword');
  obj.type = "password";
}