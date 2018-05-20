$(document).ready(function() {


  //原照片,目标
  var arr = ['content', 'style'];

  //定义每张图片的个数
  var img_count = ['50', '50'];
  var _width = parseInt(img_count[1]) * 170;
  $('.style').attr('style', 'width:'+_width+'px !important');

  for( var i = 0; i < arr.length; i++ ){

    var name = arr[i];

    var html = '';

    //原照片,目标 12张
    for( var j = 1; j <= img_count[i]; j++ ){

      //默认第一张图片被选中
      var  img_checked = parseInt(j) == 1 ?  'class="checked"' : '';
      var j = j < 10 ? '0'+ j : j;

      html += '<img src="./images/' + name + '/' + j + '.png" data-id="' + j + '" ' + img_checked + '>';
    }
    $('.' + name).html(html);
  }

  //点击图片
  $(document).on('click', '.content img, .style img', function(){

    //图片被选择中
    $(this).addClass('checked').siblings().removeClass('checked');

    //图片合成
    var content_id = $('.content .checked').data('id');
    var style_id = $('.style .checked').data('id');
    if(content_id && style_id){
      var id = content_id + '_' + style_id;
      var html = '<img src="./images/stylized/'+id + '.png" alt="' + id + '">';
      $('.stylized').html(html);
    }

  }) 

  //点击原照片第一张图片
  $('.content img').eq(0).trigger('click');
})
