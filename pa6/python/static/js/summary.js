function getSummary(page_id) {
    if ($("#sum"+page_id).is(":visible")) {
        $("#sum"+page_id).hide();
        $("#btn"+page_id).text("Show");
    } else {
        $.get(
            "/sksnf4gf1or/pa6/getdetail",
            {page_id: page_id},
            function (data, status) {
                $("#sum"+page_id).show();
                $("#img"+page_id).attr("src", data.imageUrl);
                $("#cate"+page_id).text(data.categories);
                $("#sumtext"+page_id).text(data.summary);
                $("#btn"+page_id).text("Hide");
            }
        );
    }
}
