$(document).ready(function()
{
        $("#hopstats").tablesorter({
                        headers:
                        {
                                0 : { sorter: "text" },
                                1 : { sorter: "text" },
                                2 : { sorter: "digit" },
                                3 : { sorter: "digit" },
                                4 : { sorter: "digit" },
                                5 : { sorter: "digit" },
                                6 : { sorter: "digit" },
                                7 : { sorter: "digit" },
                                8 : { sorter: "digit" },
                                9 : { sorter: "digit" },
                                10 : { sorter: "digit" },
                                11 : { sorter: "digit" }
                        },

        });
});

$(function() {
  var theTable = $('table.tablesorter')

  theTable.find("tbody > tr").find("td:eq(1)").mousedown(function(){
    $(this).prev().find(":checkbox").click()
  });

  $("#filter").keyup(function() {
    $.uiTableFilter( theTable, this.value );
  })

  $('#filter-form').submit(function(){
    theTable.find("tbody > tr:visible > td:eq(1)").mousedown();
    return false;
  }).focus();
});
