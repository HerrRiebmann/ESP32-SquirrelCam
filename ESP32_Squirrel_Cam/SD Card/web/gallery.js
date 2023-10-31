var previous;
var next;
var gallery;
var pageIndicator;
var galleryDots;
var pages;

var images= [];
var perPage = 8;
var page = 1;

// Gallery dots
function Initialize(){
	previous = document.getElementById('btnPrevious');
	next = document.getElementById('btnNext');
	gallery = document.getElementById('image-gallery');
	pageIndicator = document.getElementById('page');
	galleryDots = document.getElementById('gallery-dots');
	document.getElementById('loadingCounter').style.display = "none";

	pages = Math.ceil(images.length / perPage);
	
	for (var i = 0; i < pages; i++){
	  var dot = document.createElement('button');
	  var dotSpan = document.createElement('span');
	  var dotNumber = document.createTextNode(i + 1);
	  dot.classList.add('gallery-dot');
	  dot.setAttribute('data-index', i);
	  dotSpan.classList.add('sr-only');
	  
	  dotSpan.appendChild(dotNumber);
	  dot.appendChild(dotSpan);
	  
	  dot.addEventListener('click', function(e) {
		var self = e.target;
		goToPage(self.getAttribute('data-index'));
	  });
	  
	  galleryDots.appendChild(dot);
	}
	
	// Next Button
	next.addEventListener('click', function() {
	  if (page < pages) {
		page++;
		showImages();
	  }
	});
	
	// Previous Button
	previous.addEventListener('click', function() {
	  if (page === 1) {
		page = 1;
	  } else {
		page--;
		showImages();
	  }
	});
	showImages();	
}

// Jump to page
function goToPage(index) {
  index = parseInt(index);
  page =  index + 1;
  
  showImages();
}

// Load images
function showImages() {	
  while(gallery.firstChild)
	  gallery.removeChild(gallery.firstChild);
  
  var offset = (page - 1) * perPage;
  var dots = document.querySelectorAll('.gallery-dot');
  
  for (var i = 0; i < dots.length; i++){
    dots[i].classList.remove('active');
  }
  
  dots[page - 1].classList.add('active');
  
  for (var i = offset; i < offset + perPage; i++) {
    if ( images[i] ) {
      var template = document.createElement('div');
	  var closeBtn = document.createElement('button');
	  closeBtn.setAttribute('type', 'button');
	  closeBtn.setAttribute('onclick', 'DeleteImage(' + i + ')');	
	  closeBtn.setAttribute('class', 'img_wrp');	  
	  var closeSpan = document.createElement('span');	  
	  closeSpan.textContent = 'X';
	  closeBtn.appendChild(closeSpan);
      var title = document.createElement('p');
      var titleText = document.createTextNode(images[i].title);
      var img = document.createElement('img');
	  var aHref = document.createElement('a');
	  aHref.setAttribute('href', images[i].source);
	  aHref.setAttribute('target', '_blank');
	        
      template.classList.add('template');
      img.setAttribute("src", images[i].source);
      img.setAttribute('alt', images[i].title);

      title.appendChild(titleText);
	  aHref.appendChild(img);
      //template.appendChild(img);
	  template.appendChild(aHref);
      template.appendChild(title);
	  template.appendChild(closeBtn);
      gallery.appendChild(template);      
    }
  }
  
  // Animate images
  var galleryItems = document.querySelectorAll('.template');
  for (var i = 0; i < galleryItems.length; i++) {
    var onAnimateItemIn = animateItemIn(i);
    setTimeout(onAnimateItemIn, i * 100);
  }
  
  function animateItemIn(i) {
    var item = galleryItems[i];
    return function() {
      item.classList.add('animate');
    };
  }
  
  // Update page indicator
  pageIndicator.textContent = "Page " + page + " of " + pages;
  
}

// Function to fetch image URLs from a web service
function fetchImages() {	
	document.getElementById('loadingCounter').style.display = "block";
	var oRequest = new XMLHttpRequest();
	var sURL  = '/images';	
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			var arr = oRequest.responseText.split("|");
			arr.sort();
			for (var i = 0; i < arr.length; i++) {
			  images.push({
				title: arr[i].replace("/Photos/picture_", "").replace(".jpg", ""),
				source: arr[i]
			  });
			}
		  Initialize();
		}
	};
	oRequest.onerror = function (e) {
		document.getElementById('loadingCounter').innerHTML = "Error fetching images!";
	};
	oRequest.onprogress = function (e) {
		const headerLen = 279;
		const imgLen = 39;
		if(e.loaded < headerLen)
			return;
	  var imageCounter = Math.round((e.loaded - headerLen) / imgLen);
	  var loadingCounter = document.getElementById('loadingCounter');
	  loadingCounter.innerHTML = imageCounter;
	};
	oRequest.send(null);
}
function DeleteImage(imageNo) {	
	var oRequest = new XMLHttpRequest();
	var sURL  = '?FN=/' + images[imageNo].source + '&DEL=1&DIR=//Photos&SKIP=1';
	console.log('Delete: ' + sURL);
	oRequest.open("GET",sURL,true);
	var galleryItemNo = -1;
	for (var j = 0; j < gallery.children.length; j++){
		if(gallery.children[j].innerHTML.includes(images[imageNo].source))
		{
			galleryItemNo = j;
			break;
		}
	}
	
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){		  
		  if(galleryItemNo >= 0)
			gallery.removeChild(gallery.children[galleryItemNo]);
		  //images.splice(imageNo, 1);
		}
	};
	oRequest.onerror = function (e) {
		
	};
	oRequest.send(null);
}