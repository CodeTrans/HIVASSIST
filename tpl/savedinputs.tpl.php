<?php

  drupal_set_breadcrumb(array(l('Home', '<front>'), l('Tool', 'tool')));
  drupal_set_title($user->name. '\'s Saved Inputs');

  print $results['savedinputs_table'];
  
  