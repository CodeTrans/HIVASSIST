<?php
  drupal_set_breadcrumb(array(l('Home', '<front>'), l('Tool', 'tool')));
  drupal_set_title('HIV-ASSIST Results');
?>
<!-- Nav tabs -->
<ul class="nav nav-tabs" role="tablist">
  <li class="active"><a href="#expert" role="tab" data-toggle="tab">Expert Guidance</a></li>
  <li><a href="#ias" role="tab" data-toggle="tab">IAS Guidelines</a></li>
  <li><a href="#dhhs" role="tab" data-toggle="tab">DHHS Guidelines</a></li>
  <li class="dropdown">
    <a class="dropdown-toggle" data-toggle="dropdown" href="#">Extras <b class="caret"></b></a>
    <ul class="dropdown-menu">
      <li><a href="#mutations" role="tab" data-toggle="tab">Mutation Penalties</a></li>
      <li><a href="#comorbidities" role="tab" data-toggle="tab">Comorbidities</a></li>
      <li><a href="#drug-interactions" role="tab" data-toggle="tab">Drug Interactions</a></li>
    </ul>
  </li>
  <li><a href="#feedback" role="tab" data-toggle="tab">Feedback</a></li>
</ul>

<!-- Tab panes -->
<div class="tab-content">
  <div class="tab-pane active" id="expert"><?php print $results['regimens_table']['Expert']; ?></div>
  <div class="tab-pane" id="ias"><?php print $results['regimens_table']['IAS']; ?></div>
  <div class="tab-pane" id="dhhs"><?php print $results['regimens_table']['DHHS']; ?></div>
  <div class="tab-pane" id="mutations">
    <h3>Mutation Penalties</h3>
    <?php 
    foreach ($results['mutations_table'] as $k => $v) {
      echo '<h4>'. $k .' Mutation Penalties</h4>';
      print $results['mutations_table'][$k];
    }    
    ?>
  </div>

  <div class="tab-pane" id="comorbidities">
    <h3>Comorbidity Warnings</h3>
    <?php 
    foreach ($results['comorbidities_table'] as $k => $v) {
      echo '<h4>'. $k .' Comorbidity Warnings</h4>';
      print $results['comorbidities_table'][$k];
    }
    ?>  
  </div>
  <div class="tab-pane" id="drug-interactions">
    <h3>Drug Interactions</h3>
    <?php 
    foreach ($results['interactions_table'] as $k => $v) {
      echo '<h4>'. $k .' Interaction Warnings</h4>';
      print $results['interactions_table'][$k];
    }
    ?>    
  </div> 
  <div class="tab-pane" id="feedback">
    <?php
      $block = module_invoke('webform', 'block_view', 'client-block-48');
      print render($block['content']);
    ?>
  </div>
</div>

<div class="modal fade" id="infosheet-modal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel">
  <div class="modal-dialog modal-lg" role="document">
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="infosheet-modal-title"></h4>
      </div>
      <div class="modal-body">
    		<ul class="nav nav-tabs" id="tab-info-sheet" role="tablist">
    			<li class="active" id="nav-results-info-sheet"><a href="#results-info-sheet" data-toggle="tab">Info Sheet</a></li>
    			<li id="nav-results-comorbidities"><a href="#results-dosing-info" data-toggle="tab">Dosing Info</a></li>
          <li id="nav-results-debug"><a href="#results-debug" data-toggle="tab">Debug</a></li>
          <li id="nav-results-feedback"><a href="#results-feedback" data-toggle="tab">Feedback</a></li>
    		</ul>
    	  <div class="tab-content">
    	  	<div role="tabpanel" class="tab-pane active" id="results-info-sheet"></div>
    	  	<div role="tabpanel" class="tab-pane" id="results-dosing-info">
        		<h4>Standard Dosing</h4>
        		<div id="infosheet-dosing"></div>
        		<h4>Comorbidities</h4>
        		<div id="infosheet-comorbidities"></div>
        		<h4>Drug Interactions</h4>
        		<div id="infosheet-interactions"></div>
        		<h4>ART Interactions</h4><div id="infosheet-artinteractions"></div>
          </div>
    	  	<div role="tabpanel" class="tab-pane" id="results-debug"></div>
          <div role="tabpanel" class="tab-pane" id="results-feedback">
            <?php
              $block = module_invoke('webform', 'block_view', 'client-block-48');
              print render($block['content']);
            ?>
          </div>
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
      </div>
    </div>
  </div>
</div>


<?php


?>

