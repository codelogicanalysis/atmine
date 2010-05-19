/* Copyright (c) 2009 Domenico Gigante (http://scripts.reloadlab.net)
 * Thanks to: http://fromvega.com/scripts for drag effect.
 * Thanks for the community that is helping the improvement
 * of this little piece of code.
 *
 * Version: 1.2
 * Date: 15th Dec, 2009
 * 
 * Requires: jQuery 1.3.2+, jquery.mousewheel.js 3.0.2+, jquery.wresize.js 1.1+
 */

(function ($) {
	
	// COLLECTION OF OBJECT ELEMENTS
	var _that = {};
	
	// ON FOCUS
	var _isOnFocus;
	
	// SELECTABLE
	var _isSelectable;
	var _selectDirection;
	
	// INTERVAL
	var _intervalid;
	
	///////////////////////
	// TEMPLATE ///////////
	///////////////////////
				
	// HTML CONTAINER TEMPLATE
	var _containerHTML = '<div class="customScrollerContainer">';
	_containerHTML += '<div class="customScrollerContent">';
	_containerHTML += '</div></div>';
	
	// HTML VERTICAL SCROLLER TEMPLATE
	var _vscrollerHTML = '<div class="divVScrollerBar">';
	_vscrollerHTML += '<span class="divVScrollerBarUp"><span></span></span>';
	_vscrollerHTML += '<span class="divVScrollerBarTrace">';
	_vscrollerHTML += '<span class="divVScrollerBarCursor"><span></span></span>';
	_vscrollerHTML += '</span>';
	_vscrollerHTML += '<span class="divVScrollerBarDown"><span></span></span>';
	_vscrollerHTML += '</div>';
	
	// HTML HORIZONTAL SCROLLER TEMPLATE
	var _oscrollerHTML = '<div class="divOScrollerBar">';
	_oscrollerHTML += '<span class="divOScrollerBarLeft"><span></span></span>';
	_oscrollerHTML += '<span class="divOScrollerBarTrace">';
	_oscrollerHTML += '<span class="divOScrollerBarCursor"><span></span></span>';
	_oscrollerHTML += '</span>';
	_oscrollerHTML += '<span class="divOScrollerBarRight"><span></span></span>';
	_oscrollerHTML += '<div style="clear: both;"></div></div>';
				
	// MAIN PLUGIN (SCROLLER SET)
	$.fn.customScroller = function (options) {
		
		// DEFAULT SETTINGS
		options = $.extend({
			width: null,
			height: null,
			horizontal: 1,
			vertical: 1,
			speed: 4
		}, options);
		// INIT SCROLLBARS
		if (this.length > 0) {
			
			this.each(function (index, domElement) {
				
				var _curDate = new Date().getTime();
				
				// IF NO ID IS DEFINED ASSIGN A UNIQUE ONE
				if (undefined === this.id || !this.id.length) this.id = "customScroller_" + _curDate;
				
				// ID UNIQUE
				var _uniqueID = this.id;
				
				// THAT UNIQUE EXISTS?
				if (!_that[_uniqueID]) _that[_uniqueID] = {};
				
				// THIS REFERRER
				_that[_uniqueID].div = this;
				
				// Breite und Hoehe setzen, wenn sie nicht schon vorhanden ist. Bugfix fuer IE,
				// da dort nicht ausgewiesene Container die Breite auto bekommen
				// Dies fuehrt zu einem Fehler im Scrollplugin
				if($(this).css('width') == 'auto' && $(this).width() > 0) {
					$(this).css('width', $(this).width() + 'px');
				}
				if($(this).css('height') == 'auto' && $(this).height() > 0) {
					$(this).css('height', $(this).height() + 'px');
				}
				
				// GET DIV WIDTH
				if (!options.width) {
					if ($(this).css('width') !== 'auto') {
						_that[_uniqueID].width = ($(this).css('width').indexOf('%') > 0)? '100%': parseInt($(this).css('width'), 10) + 'px';
					} else {
						// PROBLEM IF NO WIDTH IS SET
						return false;
					}
				} else {
					_that[_uniqueID].width = ((options.width + '').indexOf('%') > 0 || (options.width + '').indexOf('px') > 0)? options.width: parseInt(options.width, 10) + 'px';
					$(this).css('width', _that[_uniqueID].width);
				}
				
				// GET DIV HEIGHT
				if (!options.height) {
					if ($(this).css('height') !== 'auto') {
						_that[_uniqueID].height = ($(this).css('height').indexOf('%') > 0)? '100%': parseInt($(this).css('height'), 10) + 'px';
					} else {
						// PROBLEM IF NO HEIGHT IS SET
						return false;
					}
				} else {
					_that[_uniqueID].height = ((options.height + '').indexOf('%') > 0 || (options.height + '').indexOf('px') > 0)? options.height: parseInt(options.height, 10) + 'px';
					$(this).css('height', _that[_uniqueID].height);
				}
				
				// SET THIS OVERFLOW HIDDEN
				$(this).css("overflow", "hidden");
				
				// SET THIS POSITION RELATIVE (FOR IE)
				$(this).css("position", "relative");
				
				// SET SPEED SCROLL (PX/20 MILLISECONDI)
				_that[_uniqueID].speed = (!$(this).attr('speed'))? options.speed: parseInt($(this).attr('speed'), 10);
				if (isNaN(_that[_uniqueID].speed)) {
					_that[_uniqueID].speed = options.speed;
				}
				
				// NO VERTICAL SCROLL
				_that[_uniqueID].vertical = (!$(this).attr('vertical'))? options.vertical: parseInt($(this).attr('vertical'), 10);
				if (isNaN(_that[_uniqueID].vertical)) {
					_that[_uniqueID].vertical = options.vertical;
				}

				// NO HORIZONTAL SCROLL
				_that[_uniqueID].horizontal = (!$(this).attr('horizontal'))? options.horizontal: parseInt($(this).attr('horizontal'), 10);
				if (isNaN(_that[_uniqueID].horizontal)) {
					_that[_uniqueID].horizontal = options.horizontal;
				}
				
				// WRAP THIS CONTENT INSIDE CONTAINER TEMPLATE
				$(this).wrapInner(_containerHTML);
				
				// CONTAINER AND CONTENT REFERRER
				_that[_uniqueID].objContainer = $(".customScrollerContainer", this);
				_that[_uniqueID].objContent = $(".customScrollerContent", this);
				
				// SET CONTAINER CSS PROPERTY
				_that[_uniqueID].objContainer.css({
												  position: 'relative',
												  float: 'left',
												  width: '100%',
												  height: '100%',
												  overflow: 'hidden',
												  margin: '0px',
												  border: '0px',
												  padding: '0px'
												  });
				// SET CONTENT CSS PROPERTY
				_that[_uniqueID].objContent.css({
												  position: 'absolute',
												  top: '0px',
												  left: '0px',
												  margin: '0px',
												  border: '0px',
												  padding: '0px'
												  });
				
				// PREPEND VERTICAL SCROLLER
				$(this).prepend(_vscrollerHTML);
				
				// VERTICAL SCROLLER OBJECTS REFERRER
				_that[_uniqueID].objVScroller = $(".divVScrollerBar", this);
				_that[_uniqueID].objUp = $(".divVScrollerBarUp", this);
				_that[_uniqueID].objDown = $(".divVScrollerBarDown", this);
				_that[_uniqueID].objVTrace = $(".divVScrollerBarTrace", this);
				_that[_uniqueID].objVCursor = $(".divVScrollerBarCursor", this);
					
				// SET VERTICAL SCROLLER CSS PROPERTY
				_that[_uniqueID].objVScroller.css({
												  float: 'right',
												  overflow: 'hidden',
												  padding: '0px'
												  });
				_that[_uniqueID].objUp.css({
												  display: 'block',
												  width: '100%',
												  overflow: 'hidden'
												  });
				_that[_uniqueID].objDown.css({
												  display: 'block',
												  width: '100%',
												  overflow: 'hidden'
												  });
				_that[_uniqueID].objVTrace.css({
												  display: 'block',
												  position: 'relative',
												  width: '100%',
												  overflow: 'hidden',
												  margin: '0px',
												  border: '0px',
												  padding: '0px'
												  });
				_that[_uniqueID].objVCursor.css({
												  display: 'block',
												  position: 'absolute',
												  width: '100%',
												  overflow: 'hidden',
												  top: '0px',
												  left: '0px',
												  margin: '0px',
												  border: '0px',
												  padding: '0px'
												  });
				
				// APPEND HORIZONTAL SCROLLER
				$(this).append(_oscrollerHTML);
					
				// HORIZONTAL SCROLLER OBJECTS REFERRER
				_that[_uniqueID].objOScroller = $(".divOScrollerBar", this);
				_that[_uniqueID].objLeft = $(".divOScrollerBarLeft", this);
				_that[_uniqueID].objRight = $(".divOScrollerBarRight", this);
				_that[_uniqueID].objOTrace = $(".divOScrollerBarTrace", this);
				_that[_uniqueID].objOCursor = $(".divOScrollerBarCursor", this);
				
				// SET HORIZONTAL SCROLLER CSS PROPERTY
				_that[_uniqueID].objOScroller.css({
												  float: 'left',
												  overflow: 'hidden',
												  padding: '0px'
												  });
				_that[_uniqueID].objLeft.css({
												  float: 'left',
												  display: 'block',
												  height: '100%',
												  margin: '0px',
												  overflow: 'hidden'
												  });
				_that[_uniqueID].objRight.css({
												  float: 'left',
												  display: 'block',
												  height: '100%',
												  margin: '0px',
												  overflow: 'hidden'
												  });
				_that[_uniqueID].objOTrace.css({
												  float: 'left',
												  display: 'block',
												  position: 'relative',
												  height: '100%',
												  overflow: 'hidden',
												  margin: '0px',
												  border: '0px',
												  padding: '0px'
												  });
				_that[_uniqueID].objOCursor.css({
												  display: 'block',
												  position: 'absolute',
												  height: '100%',
												  overflow: 'hidden',
												  top: '0px',
												  left: '0px',
												  margin: '0px',
												  border: '0px',
												  padding: '0px'
												  });
				
				// END OF SCROLLER
				$(this).append('<div style="clear: both;"></div>');
				// FUNCTION TO INIT AND CHANGE SCROLLER
				$.fn.setCustomScroller = function (initPosition) {
					
					if (this.length > 0) {
						
						this.each(function (index, domElement) {
							
							if (this.id !== undefined && this.id.length > 0) {
								
								var id = this.id;
								
								if (_that[id] && $(_that[id].div).length > 0) {
										
									// SET INIT STATUS VERTICAL AND HORIZONTAL
									_that[id]._vscroll = false;
									_that[id]._oscroll = false;
									
									if(_that[id].objOScroller != undefined) {
										_that[id].objOScroller.hide();
									}
									if(_that[id].objVScroller != undefined) {
										_that[id].objVScroller.hide();
									}
									
									_that[id].objContainer.css({
															   width: '100%',
															   height: '100%'
															   });
									
									// IE BUG FIX (TEXT NO FILL CONTENT BOX WITH IMAGE)
									if ($.browser.msie) {
										_that[id].objContent.css({
																   width: 'auto'
																   });
									}
									
									// IF CONTENT HEIGHT > CONTAINER HEIGHT, THEN...
									if (_that[id].vertical === 1 && _that[id]._vscroll === false && _that[id].objContainer.height() > 0 && _that[id].objContent.outerHeight(true) > _that[id].objContainer.height()) {
										
										// SET VERTICAL SCROLLER EXIST
										_that[id]._vscroll = true;
										
										// SHOW VERTICAL SCROLLER
										_that[id].objVScroller.show();
									
										// SET CONTAINER WIDTH
										_that[id].objContainer.css('width', (_that[id].objContainer.width() - _that[id].objVScroller.outerWidth(true) - 1) + 'px');
										
										// GET CONTAINER AND CONTENT HEIGHT
										_that[id].containerHeight = _that[id].objContainer.height();
										_that[id].contentHeight = _that[id].objContent.outerHeight(true);
										
										// Check Contentposition, reposition if needed
										var visibleHeight = _that[id].contentHeight - (_that[id].objContainer.offset().top - _that[id].objContent.offset().top);
										if(visibleHeight < _that[id].containerHeight) {
											var newContentTop = parseInt(_that[id].objContent.css('top'), 10) + (_that[id].containerHeight - visibleHeight);
											_that[id].objContent.css('top', newContentTop);
										}
										
										if(initPosition === true) {
											_that[id].objContent.css('top', 0);
										}
										
										// SET SCROLLER HEIGHT
										_that[id].objVScroller.css('height', _that[id].containerHeight + 'px');
										
										// ESTIMATE TRACE HEIGHT
										_that[id].traceHeight = _that[id].containerHeight - _that[id].objUp.outerHeight(true) - _that[id].objDown.outerHeight(true);
										
										// SET TRACE HEIGHT
										_that[id].objVTrace.css('height', _that[id].traceHeight + 'px');
										
										// ESTIMATE CURSOR HEIGHT
										_that[id].cursorHeight = Math.ceil((_that[id].traceHeight * _that[id].containerHeight) / _that[id].contentHeight);
										
										// SET CURSOR HEIGHT
										_that[id].objVCursor.css('height', _that[id].cursorHeight + 'px');
										
										// DIFFERENCE BETWEEN TRACE HEIGHT AND CURSOR HEIGHT
										_that[id].traceVVoid = _that[id].traceHeight - _that[id].cursorHeight;
										
										// ACTUAL TRACE POSITION TOP
										_that[id].posVTrace = _that[id].objVTrace.offset().top;
										
										// SET CURSOR TOP POSITION
										var cursorY = (0 - parseInt(_that[id].objContent.css('top'), 10)) * _that[id].traceVVoid / (_that[id].contentHeight - _that[id].containerHeight);
										
										_that[id].objVCursor.css('top', cursorY + 'px');
										
										
										
									}
									
									// IF CONTENT WIDTH > CONTAINER WIDTH, THEN...
									if (_that[id].horizontal === 1 && _that[id]._oscroll === false && _that[id].objContainer.width() > 0 && _that[id].objContent.outerWidth(true) > _that[id].objContainer.width()) {
					
										// SET HORIZONTAL SCROLLER EXIST
										_that[id]._oscroll = true;
										
										// SHOW HORIZONTAL SCROLLER
										_that[id].objOScroller.show();
						
										// SET CONTAINER HEIGHT
										_that[id].objContainer.css('height', (_that[id].objContainer.height() - _that[id].objOScroller.outerHeight(true)) + 'px');
										
										// GET CONTAINER AND CONTENT WIDTH
										_that[id].containerWidth = _that[id].objContainer.width();
										_that[id].contentWidth = _that[id].objContent.outerWidth(true);
							
										// SET SCROLLER WIDTH
										_that[id].objOScroller.css('width', _that[id].containerWidth + 'px');
										
										// ESTIMATE TRACE WIDTH
										_that[id].traceWidth = _that[id].containerWidth - _that[id].objLeft.outerWidth(true) - _that[id].objRight.outerWidth(true);
										
										// SET TRACE WIDTH
										_that[id].objOTrace.css('width', _that[id].traceWidth + 'px');
										
										// ESTIMATE CURSOR WIDTH
										_that[id].cursorWidth = Math.ceil((_that[id].traceWidth * _that[id].containerWidth) / _that[id].contentWidth);
										
										// SET CURSOR WIDTH
										_that[id].objOCursor.css('width', _that[id].cursorWidth + 'px');
										
										// DIFFERENCE BETWEEN TRACE WIDTH AND CURSOR WIDTH
										_that[id].traceOVoid = _that[id].traceWidth - _that[id].cursorWidth;
										
										// ACTUAL TRACE POSITION LEFT
										_that[id].posOTrace = _that[id].objOTrace.offset().left;
										
										// SET CURSOR LEFT POSITION
										var cursorX = (0 - parseInt(_that[id].objContent.css('left'), 10)) * _that[id].traceOVoid / (_that[id].contentWidth - _that[id].containerWidth);
										_that[id].objOCursor.css('left', cursorX + 'px');
										
										// IF VERTICAL SCROLLER EXIST, THEN...
										if (_that[id].vertical === 1 && _that[id]._vscroll === true) {
											
											// GET CONTAINER AND CONTENT HEIGHT
											_that[id].containerHeight = _that[id].objContainer.height();
											_that[id].contentHeight = _that[id].objContent.outerHeight(true);
										
											// SET SCROLLER HEIGHT
											_that[id].objVScroller.css('height', _that[id].containerHeight + 'px');
											
											// ESTIMATE TRACE HEIGHT
											_that[id].traceHeight = _that[id].containerHeight - _that[id].objUp.outerHeight(true) - _that[id].objDown.outerHeight(true);
											
											// SET TRACE HEIGHT
											_that[id].objVTrace.css('height', _that[id].traceHeight + 'px');
											
											// ESTIMATE CURSOR HEIGHT
											_that[id].cursorHeight = Math.ceil((_that[id].traceHeight * _that[id].containerHeight) / _that[id].contentHeight);
											
											// SET CURSOR HEIGHT
											_that[id].objVCursor.css('height', _that[id].cursorHeight + 'px');
											
											// DIFFERENCE BETWEEN TRACE HEIGHT AND CURSOR HEIGHT
											_that[id].traceVVoid = _that[id].traceHeight - _that[id].cursorHeight;
											
											// ACTUAL TRACE POSITION TOP
											_that[id].posVTrace = _that[id].objVTrace.offset().top;
											
											// SET CURSOR TOP POSITION
											var cursorY = (0 - parseInt(_that[id].objContent.css('top'), 10)) * _that[id].traceVVoid / (_that[id].contentHeight - _that[id].containerHeight);
											_that[id].objVCursor.css('top', cursorY + 'px');
								
										}
										
									}
									
									// IE BUG FIX (TEXT NO FILL CONTENT BOX WITH IMAGE)
									if ($.browser.msie) {
										_that[id].objContent.css({
																   width: _that[id].objContainer.width() + 'px'
																   });
									}
									
									// IF NO SCROLLER, THEN..
									if (_that[id]._vscroll === false) {
										_that[id].objContent.css('top', '0px');
									}
									if (_that[id]._oscroll === false) {
										_that[id].objContent.css('left', '0px');
									}
									
									// GET CONTAINER OFFSET
									_that[id].containerOffset = _that[id].objContainer.offset();
									
									// GET CONTAINER AND CONTENT HEIGHT
									_that[id].containerHeight = _that[id].objContainer.height();
									_that[id].contentHeight = _that[id].objContent.outerHeight();
									
									// GET CONTAINER AND CONTENT WIDTH
									_that[id].containerWidth = _that[id].objContainer.width();
									_that[id].contentWidth = _that[id].objContent.outerWidth(true);
									
									// MOUSEWHEEL EVENT
									if ($().mousewheel) {
										if (_that[id]._vscroll === true) {
											// ONMOUSEWHEEL VERTICAL
											_that[id].objContainer.unbind('mousewheel').bind('mousewheel', function (e, delta) {
												(delta > 0)?_moveUp(_that[id], delta):_moveDown(_that[id], delta);
												
												return false;
											});
										} else {
											// ONMOUSEWHEEL HORIZONTAL
											_that[id].objContainer.unbind('mousewheel').bind('mousewheel', function (e, delta) {
												(delta > 0)?_moveLeft(_that[id], delta):_moveRight(_that[id], delta);
												
												return false;
											});
										}
									}
									
								}
							
							}
						
						});
					
					}
				
				};
				
				// INIT SET SCROLLER
				$(this).setCustomScroller();
				
				
				/////////////////////
				// EVENTS ///////////
				/////////////////////
				
				// ONMOUSEDOWN SET FOCUS ON ELEMENT
				$(this).bind('mousedown', function (e) {
					e.stopPropagation();
					_isOnFocus = _uniqueID;
				});
					
				// ONMOUSEDOWN SET SELECTABLE ON ELEMENT
				_that[_uniqueID].objContainer.bind('mousedown', function (e) {
					_isSelectable = _uniqueID;
					_selectDirection = null;
				});
				
				// ONMOUSEMOVE SELECT CONTENT
				_that[_uniqueID].objContainer.bind('mousemove', function (e) {
	
					if (!_isSelectable || !_that[_isSelectable] || !$(_that[_isSelectable].div).length) return;
						
					var containerOffset = _that[_isSelectable].containerOffset;
					var containerHeight = _that[_isSelectable].containerHeight;
					var containerWidth = _that[_isSelectable].containerWidth;
						
					_stopMove();
						
					if (_that[_isSelectable]._vscroll === true && e.pageY > containerOffset.top && e.pageY < containerOffset.top + 10) {
						_selectDirection = 'up';
						_startMoveUp(_that[_isSelectable], 1);
					} else if (_that[_isSelectable]._oscroll === true && e.pageX > containerOffset.left && e.pageX < containerOffset.left + 10) {
						_selectDirection = 'left';
						_startMoveLeft(_that[_isSelectable], 1);
					} else if (_that[_isSelectable]._vscroll === true && e.pageY > (containerOffset.top + containerHeight - 10) && e.pageY < (containerOffset.top + containerHeight)) {
						_selectDirection = 'down';
						_startMoveDown(_that[_isSelectable], -1);
					} else if (_that[_isSelectable]._oscroll === true && e.pageX > (containerOffset.left + containerWidth - 10) && e.pageX < (containerOffset.left + containerWidth)) {
						_selectDirection = 'right';
						_startMoveRight(_that[_isSelectable], -1);
					}
				});
				
				// ONMOUSEUP UNSET SELECTABLE ON ELEMENT
				_that[_uniqueID].objContainer.bind('mouseup', function (e) {
					_stopMove();
					_isSelectable = null;
					_selectDirection = null;
				});
				
				// VERTICAL SCROLLER EVENTS
				_that[_uniqueID].objVTrace.bind("mousedown", function (e) {
					var spanY = (e.pageY - _that[_uniqueID].posVTrace);
					if (spanY > (_that[_uniqueID].cursorHeight + parseInt(_that[_uniqueID].objVCursor.css('top'), 10))) {
						_moveDown(_that[_uniqueID], -3);
					} else if (spanY < parseInt(_that[_uniqueID].objVCursor.css('top'), 10)) {
						_moveUp(_that[_uniqueID], 3);
					}
					
					return false;
				});
					
				_that[_uniqueID].objUp.bind("mouseover", function (e) {
					$('span', this).addClass('hover');
					_startMoveUp(_that[_uniqueID]);
					
					return false;
				});
					
				_that[_uniqueID].objDown.bind("mouseover", function (e) {
					$('span', this).addClass('hover');
					_startMoveDown(_that[_uniqueID]);
					
					return false;
				});
					
				_that[_uniqueID].objUp.bind("mouseout", function (e) {
					$('span', this).removeClass('hover');
					_stopMove();
					
					return false;
				});
					
				_that[_uniqueID].objDown.bind("mouseout", function (e) {
					$('span', this).removeClass('hover');
					_stopMove();
					
					return false;
				});
					
				_that[_uniqueID].objVCursor.bind("mouseover", function (e) {
					$('span', this).addClass('hover');
					
					return false;
				});

				_that[_uniqueID].objVCursor.bind("mouseout", function (e) {
					$('span', this).removeClass('hover');
					
					return false;
				});

				// SET VERTICAL CURSOR DRAGGABLE
				$(_that[_uniqueID].objVCursor).dragCursor(_uniqueID, 'bottom');
				
				$(_that[_uniqueID].objVCursor).ondrag(function (e, element) { 
					var cursorY = parseInt(_that[_uniqueID].objVCursor.css('top'), 10);
					var contentY = 0 - (cursorY * (_that[_uniqueID].contentHeight - _that[_uniqueID].containerHeight) / _that[_uniqueID].traceVVoid);
					_that[_uniqueID].objContent.css('top', contentY + "px");
					
					return false;
				});
				
				// HORIZONTAL SCROLLER EVENTS
				_that[_uniqueID].objOTrace.bind("mousedown", function (e) {
					var spanX = (e.pageX - _that[_uniqueID].posOTrace);
					if (spanX > (_that[_uniqueID].cursorWidth + parseInt(_that[_uniqueID].objOCursor.css('left'), 10))) {
						_moveRight(_that[_uniqueID], -3);
					} else if (spanX < parseInt(_that[_uniqueID].objOCursor.css('left'), 10)) {
						_moveLeft(_that[_uniqueID], 3);
					}
					
					return false;
				});
					
				_that[_uniqueID].objLeft.bind("mouseover", function (e) {
					$('span', this).addClass('hover');
					_startMoveLeft(_that[_uniqueID]);
					
					return false;
				});
					
				_that[_uniqueID].objRight.bind("mouseover", function (e) {
					$('span', this).addClass('hover');
					_startMoveRight(_that[_uniqueID]);
					
					return false;
				});
					
				_that[_uniqueID].objLeft.bind("mouseout", function (e) {
					$('span', this).removeClass('hover');
					_stopMove();
					
					return false;
				});
					
				_that[_uniqueID].objRight.bind("mouseout", function (e) {
					$('span', this).removeClass('hover');
					_stopMove();
					
					return false;
				});
					
				_that[_uniqueID].objOCursor.bind("mouseover", function (e) {
					$('span', this).addClass('hover');
					
					return false;
				});

				_that[_uniqueID].objOCursor.bind("mouseout", function (e) {
					$('span', this).removeClass('hover');
					
					return false;
				});

				// SET HORIZONTAL CURSOR DRAGGABLE
				$(_that[_uniqueID].objOCursor).dragCursor(_uniqueID, 'right');
				
				$(_that[_uniqueID].objOCursor).ondrag(function (e, element) { 
					var cursorX = parseInt(_that[_uniqueID].objOCursor.css('left'), 10);
					var contentX = 0 - (cursorX * (_that[_uniqueID].contentWidth - _that[_uniqueID].containerWidth) / _that[_uniqueID].traceOVoid);
					_that[_uniqueID].objContent.css('left', contentX + "px");
					
					return false;
				});
			});
			
			$.anchorFix();
			
		}
	};

	// ADD ANCHORS SUPPORT
	$.anchorFix = function () {
		$('a').each(function (index, domElement) {
			$(this).bind('click', function (e) {
				var getHref = $(this).attr('href');
				if (getHref && getHref.indexOf('#') !== -1) {
					var anchors = getHref.replace(window.location.href, '');
					anchors = anchors.substring(anchors.indexOf('#'));
				}
				var targets = $(this).attr('target');
				if (anchors && targets && _that[targets]) {
					if (anchors.length > 0 && anchors.charAt(0) === '#') {
						var topAnchor = $('a[name=' + anchors.substring(1) + ']').parent().offset().top;
						var topContainer = _that[targets].objContainer.offset().top;
						var topContent = parseInt(_that[targets].objContent.css('top'), 10);
										
						var offScroll = topContent + ( ( 0 - parseInt(topAnchor - topContainer, 10) ) ); 
										
						var maxScroll = ((0 - _that[targets].contentHeight) + _that[targets].containerHeight);
										
						if (offScroll < maxScroll) {
							offScroll = maxScroll;
						}
										
						_that[targets].objContent.css('top', offScroll + 'px');
								
						// set cursor top position
						var cursorY = (0 - parseInt(offScroll, 10)) * _that[targets].traceVVoid / (_that[targets].contentHeight - _that[targets].containerHeight);
						_that[targets].objVCursor.css('top', cursorY + 'px'); // imposta scrol cursore
			
						return false;
					}
				}
			});
		});
	};
	
	// All move function
	var _startMoveDown = function (objCopy, wheelDelta) {
		_intervalid = window.setInterval(function () { _moveDown(objCopy, wheelDelta); }, 20);
	};
		
	var _startMoveUp = function (objCopy, wheelDelta) {
		_intervalid = window.setInterval(function () { _moveUp(objCopy, wheelDelta); }, 20);
	};
		
	var _startMoveRight = function (objCopy, wheelDelta) {
		_intervalid = window.setInterval(function () { _moveRight(objCopy, wheelDelta); }, 20);
	};
		
	var _startMoveLeft = function (objCopy, wheelDelta) {
		_intervalid = window.setInterval(function () { _moveLeft(objCopy, wheelDelta); }, 20);
	};
		
	var _stopMove = function () {
		if (_intervalid) window.clearInterval(_intervalid);
	};
		
	var _moveDown = function (objCopy, wheelDelta) {
		var increment;
		if (wheelDelta) increment = (0 - parseInt(wheelDelta * 5, 10));
		else increment = 1;
		var scrolling = parseInt(objCopy.objContent.css('top'), 10);
		var maxScroll = ((0 - objCopy.contentHeight) + objCopy.containerHeight);
		if (scrolling >= maxScroll) {
			var delTop = scrolling - parseInt(objCopy.speed * increment, 10);
			if (delTop < maxScroll) delTop = maxScroll;
			objCopy.objContent.css('top', delTop + 'px');
			var addTop = parseInt((((0 - delTop) * objCopy.traceVVoid) / (objCopy.contentHeight - objCopy.containerHeight)), 10);
			objCopy.objVCursor.css('top', addTop + 'px');
		}
	};
		
	var _moveUp = function (objCopy, wheelDelta) {
		var increment;
		if (wheelDelta) increment = parseInt(wheelDelta * 5, 10);
		else increment = 1;
		var scrolling = parseInt(objCopy.objContent.css('top'), 10);
		if (scrolling <= 0) {
			var addTop = scrolling + parseInt(objCopy.speed * increment, 10);
			if (addTop > 0) addTop = 0;
			objCopy.objContent.css('top', addTop + 'px');
			var delTop = parseInt((((0 - addTop) * objCopy.traceVVoid) / (objCopy.contentHeight - objCopy.containerHeight)), 10);
			objCopy.objVCursor.css('top', delTop + 'px');
		}
	};
	
	var _moveRight = function (objCopy, wheelDelta) {
		var increment;
		if (wheelDelta) increment = (0 - parseInt(wheelDelta * 5, 10));
		else increment = 1;
		var scrolling = parseInt(objCopy.objContent.css('left'), 10);
		var maxScroll = ((0 - objCopy.contentWidth) + objCopy.containerWidth);
		if (scrolling >= maxScroll) {
			var delRight = scrolling - parseInt(objCopy.speed * increment, 10);
			if (delRight < maxScroll) delRight = maxScroll;
			objCopy.objContent.css('left', delRight + 'px');
			var addLeft = parseInt((((0 - delRight) * objCopy.traceOVoid) / (objCopy.contentWidth - objCopy.containerWidth)), 10);
			objCopy.objOCursor.css('left', addLeft + 'px');
		}
	};
		
	var _moveLeft = function (objCopy, wheelDelta) {
		var increment;
		if (wheelDelta) increment = parseInt(wheelDelta * 5, 10);
		else increment = 1;
		var scrolling = parseInt(objCopy.objContent.css('left'), 10);
		if (scrolling <= 0) {
			var addLeft = scrolling + parseInt(objCopy.speed * increment, 10);
			if (addLeft > 0) addLeft = 0;
			objCopy.objContent.css('left', addLeft + 'px');
			var delRight = parseInt((((0 - addLeft) * objCopy.traceOVoid) / (objCopy.contentWidth - objCopy.containerWidth)), 10);
			objCopy.objOCursor.css('left', delRight + 'px');
		}
	};
	
	// DRAG PLUGIN (CURSORS DRAGGABLE)
	$.fn.dragCursor = function (id, which) {
		
		// SOME PRIVATE VARS
		var _isMouseDown = false;
		var _currentElement = null;
		var _dragCallbacks = {};
		var _lastMouseX;
		var _lastMouseY;
		var _lastElemTop;
		var _lastElemLeft;
		if (_that[id] && $(_that[id].div).length > 0) {
			
			// DEFAUL SETTINGS
			options = {
				maxTop: 0,
				maxRight: 0,
				maxBottom: 0,
				maxLeft: 0
			};
					
			// REGISTER THE FUNCTION TO BE CALLED WHILE AN ELEMENT IS BEING DRAGGED
			$.fn.ondrag = function (callback) {
				if (this.length > 0) {
					return this.each(function () {
						_dragCallbacks[this.id] = callback;
					});
				}
			};
			
			if (this.length > 0) {
				this.each(function (index, domElement) {
					
					// WHEN AN ELEMENT RECEIVES A MOUSE PRESS
					$(this).bind("mousedown", function (e) {
											
						if (which === 'bottom') {
							options.maxTop = 0;
							options.maxRight = 0;
							options.maxBottom = _that[id].traceVVoid;
							options.maxLeft = 0;
						}
						if (which === 'right') {
							options.maxTop = 0;
							options.maxRight = _that[id].traceOVoid;
							options.maxBottom = 0;
							options.maxLeft = 0;
						}
						
						// UPDATE TRACK VARIABLES
						_isMouseDown = true;
						
						_currentElement = this;
						
						// RETRIEVE POSITIONING PROPERTIES
						var offset = $(this).offset();
						var parentOffSet = $(this).offsetParent();
						var additionalOffSet = {};
						if (parentOffSet.length > 0) {
							additionalOffSet.top = parentOffSet.offset().top;
							additionalOffSet.left = parentOffSet.offset().left;
						}
						
						// GLOBAL POSITION RECORDS
						_lastMouseX = e.pageX;
						_lastMouseY = e.pageY;
						
						_lastElemTop = offset.top - additionalOffSet.top;
						_lastElemLeft = offset.left - additionalOffSet.left;
						
						// UPDATE THE POSITION
						updatePosition(e);
						
						return false;
					});
				});
			}
			
			// UPDATES THE POSITION OF THE CURRENT ELEMENT BEING DRAGGED
			var updatePosition = function (e) {
				var spanX = (e.pageX - _lastMouseX);
				var spanY = (e.pageY - _lastMouseY);
				
				var Y = _lastElemTop + spanY;
				var X = _lastElemLeft + spanX;
				
				if (options.maxTop !== null && Y < options.maxTop) Y = options.maxTop;
				if (options.maxLeft !== null && X < options.maxLeft) X = options.maxLeft;
				if (options.maxBottom !== null && Y > options.maxBottom) Y = options.maxBottom;
				if (options.maxRight !== null && X > options.maxRight) X = options.maxRight;
						
				$(_currentElement).css("top", Y + 'px');
				$(_currentElement).css("left", X + 'px');
			};
			
			// WHEN THE MOUSE IS MOVED WHILE THE MOUSE BUTTON IS PRESSED
			$(document).bind("mousemove", function (e) {
				if (_isMouseDown === true) {
					
					// UPDATE THE POSITION
					updatePosition(e);
					
					// CALL ONDRAG FUNCTION
					if (typeof _dragCallbacks[_currentElement.id] === 'function') {
						_dragCallbacks[_currentElement.id](e, _currentElement);
					}
					
					return false;
				}
			});
			
			// WHEN THE MOUSE BUTTON IS RELEASED
			$(document).bind("mouseup", function (e) {
					
				_isMouseDown = false;
					
				return false;
			});
		
		}
	};
	
	// AJAX PLUGIN (LOADING CONTENT INTO ELEMENT AND SET SCROLLER)
	$.fn.ajaxScroller = function (url, data, options) {
		
		// DEFAUL SETTINGS
		options = $.extend({

		}, options);
		
		if (this.length > 0) {
			this.each(function (index, domElement) {
				$(this).html("Loading...");
				
				$(this).load(url, data, function (responseText, textStatus, XMLHttpRequest) {
					if (textStatus === "success") {
						$(this).customScroller(options);
					} else {
						alert('Error');
					}
				});
			});
		}
	};
	
	// ONREADY DOCUMENT EVENTS
	$(document).ready(function () { 
		
		// ONMOUSEDOWN RESET FOCUS
		$(document).bind('mousedown', function (e) {
			_isOnFocus = null;
		});
	
		// MOVE ON KEY PRESS
		$(document).bind("keydown keypress", function (e) {
			
			if (!_isOnFocus || !_that[_isOnFocus] || !$(_that[_isOnFocus].div).length) return;
	
			switch(e.which) {
				case 38: // UP
					_moveUp(_that[_isOnFocus], 1);	
				break;
							
				case 40: // DOWN
					_moveDown(_that[_isOnFocus], -1);
				break;
							
				case 37: // LEFT
					_moveLeft(_that[_isOnFocus], 1);
				break;
							
				case 39: // RIGHT
					_moveRight(_that[_isOnFocus], -1);
				break;
				
				case 33: // UP
					_moveUp(_that[_isOnFocus], 3);	
				break;
							
				case 34: // DOWN
					_moveDown(_that[_isOnFocus], -3);
				break;
							
				case 36: // LEFT
					_moveLeft(_that[_isOnFocus], 3);
				break;
							
				case 35: // RIGHT
					_moveRight(_that[_isOnFocus], -3);
				break;
				
				default:
					return true;
				break;
			}
			return false;
		});
				
		// RESIZE FRAME ON RESIZE WINDOW
		if ($().wresize) {
			$(window).wresize(function () {
				
				if (_intervalid) clearTimeout(_intervalid); 
				
				_intervalid = setTimeout(function () {
					
					$.each(_that, function (key, value) {
						
						if ($(_that[key].div).length > 0) {
							
							$(_that[key].div).setCustomScroller();
						
						}
						
					});
				
				}, 100); 
				
				return false;
			});
		}
	});
})(jQuery);
