<?php
function hivassist_saved_inputs() {
  global $user;
  
  module_load_include('inc', 'webform', '/includes/webform.submissions');

  $node = node_load('46');
  $submissions = webform_get_submissions($filters = array('nid' => '46', 'uid' => $user->uid));
  
  $header = array('Identifier', 'Options', '', '', 'Results', 'Delete');
  
//  krumo($submissions);
  
  foreach ($submissions as $k => $v) {
    (empty($submissions[$k]->data[1][0]) ? $mutations = 'None' : $mutations = $submissions[$k]->data[1][0]);
    (empty($submissions[$k]->data[3][0]) ? $comorbidities = 'None' : $comorbidities = implode(', ', $submissions[$k]->data[3]));
    (empty($submissions[$k]->data[7][0]) ? $interactions = 'None' : $interactions = implode(', ', $submissions[$k]->data[7]));
    (empty($submissions[$k]->data[10][0]) ? $history = 'None' : $history = implode(', ', $submissions[$k]->data[10]));
    (empty($submissions[$k]->data[8][0]) ? $exclude = 'None' : $exclude = implode(', ', $submissions[$k]->data[8]));

    if (webform_submission_access($node, $v, 'delete', $user)) {
      $delete = l('Delete', "node/$node->nid/submission/$k/delete", array('query' => drupal_get_destination()));
    } else {
      $delete = 'Delete';
    }

    $rows[] = array(
      'Identifier' => $submissions[$k]->data[13][0],
      'Options' => '<strong>Mutations: </strong>' .$mutations. '<br />
                    <strong>Comorbidities: </strong>' .$comorbidities. '<br />
                    <strong>Interactions: </strong>' .$interactions. '<br />',
      'More Options' => '<strong>Tropism: </strong>' .$submissions[$k]->data[11][0]. '<br />
                    <strong>Viral Load: </strong>' .$submissions[$k]->data[5][0]. '<br />
                    <strong>HLA B5701: </strong>' .$submissions[$k]->data[6][0]. '<br />
                    <strong>Poor Adherence: </strong>' .$submissions[$k]->data[9][0]. '<br />
                    <strong>Treatment History: </strong>' .$history,
      'Even More Options' => '<strong>Excluded Drugs: </strong>' .$exclude,
      'Results' => l('Results', 'tool/results', array('query' => array(
        'mutations' => $submissions[$k]->data[1][0],
        'comorbidities' => $submissions[$k]->data[3],
        'interactions' => $submissions[$k]->data[7],
        'tropism' => $submissions[$k]->data[11][0],
        'viralload' => $submissions[$k]->data[5][0],
        'hlab5701' => $submissions[$k]->data[6][0],
        'pooradherence' => $submissions[$k]->data[9][0],
        'history' => $submissions[$k]->data[10],
        'exclude' => $submissions[$k]->data[8],
      ))),
      'Delete' => $delete,
    );
  }

  if (empty($rows)) {
    $savedinputs_table = 'You do not have any saved inputs.';
  } else {
    $per_page = 20;
    $current_page = pager_default_initialize(count($rows), $per_page);
    $chunks = array_chunk($rows, $per_page, TRUE);
    $savedinputs_table = theme('table', array('header' => $header, 'rows' => $chunks[$current_page]));
    $savedinputs_table .= theme('pager', array('quantity',count($rows)));          
  }
  
  $results = array(
    'savedinputs' => $submissions,
    'savedinputs_table' => $savedinputs_table,
  );
  
  $output = theme('hivassist_savedinputs', array('results' => $results));
  return $output;
}
  