<?php

function hivassist_nonzero ($var) {
   return ($var > 0);
}

function hivassist_lowlevel ($var) {
  return ($var > 0 && $var < 30);
}

function hivassist_add_mutation_score_penalty (&$regimen, $option = NULL) {
  $values = array('XTC' => 0, 'one' => 0, 'operand' => '+');

  if ($option == 'ignore XTC') {
    if (in_array('3TC', $regimen['Combination'])): $values['XTC'] = $regimen['Mutations']['3TC'];
    elseif (in_array('FTC', $regimen['Combination'])): $values['XTC'] = $regimen['Mutations']['FTC'];
    endif;

    $values['one'] = 1;
  }

  if ((pow(2, count(array_filter($regimen['Mutations'], 'hivassist_nonzero')) - $values['one']) * 0.01 * (array_sum($regimen['Mutations']) - $values['XTC'])) >= 0): $values['operand'] = '+ ';
  else: $values['operand'] = '- ';
  endif;    
  
  $regimen['Score'] += pow(2, count(array_filter($regimen['Mutations'], 'hivassist_nonzero')) - $values['one']) * 0.01 * (array_sum($regimen['Mutations']) - $values['XTC']);
  $regimen['Score Explanation'] .= $values['operand'] .abs(pow(2, count(array_filter($regimen['Mutations'], 'hivassist_nonzero')) - $values['one']) * 0.01 * (array_sum($regimen['Mutations']) - $values['XTC'])). ' (2<sup>' .(count(array_filter($regimen['Mutations'], 'hivassist_nonzero')) - $values['one']). '</sup> * 0.01 * ' .(array_sum($regimen['Mutations']) - $values['XTC']). ' for mutations: ';
  foreach ($regimen['Mutations'] as $k => $v) {
    $regimen['Score Explanation'] .= $k. ': ' .$regimen['Mutations'][$k]. ', ';
  }
  $regimen['Score Explanation'] = substr($regimen['Score Explanation'], 0, -2). ')<br />';
 
}

function hivassist_rules (&$regimen, $regimens, $mutations, $options) {
  global $defs;

  $regimen['Score Explanation'] = 'Rule: ' .$regimen['Rule']. ' (' .$regimen['RegimenType']. ')<br />';

  $regimen['Base Score'] = $regimen['Score'];
  $regimen['Score Explanation'] .= $regimen['Base Score']. ' (Base score) <br />';
  
  // Remove MVC containing regimens if not R5 tropism
  if ($options['tropism'] != 'R5' && in_array('MVC', $regimen['Combination'])) {
    $regimen = NULL;
    return;
  }
  
  // Remove ABC containing regimens if HLA B5701 positive
  if ($options['hlab5701'] == 'positive' && in_array('ABC', $regimen['Combination'])) {
    $regimen = NULL;
    return;
  }

  $regimen['Active Drugs'] = count($regimen['Combination']) - count(array_filter($regimen['Mutations'], 'hivassist_nonzero')) + 0.5 * count(array_filter($regimen['Mutations'], 'hivassist_lowlevel'));

  // Incorporate the mutation rules:
	// If a single drug (that is not XTC) has a mutation score >= 60, that regimen automatically becomes 6.X
	// Otherwise, the regimen becomes (# of drugs with mutation score > 0) * 0.01 * sum of the mutation penalties
	if (max($regimen['Mutations']) >= 60) { // Check if any drug has a mutation score >= 60
    if ((in_array('3TC', $regimen['Combination']) && $regimen['Mutations']['3TC'] >= 60) || (in_array('FTC', $regimen['Combination']) && $regimen['Mutations']['FTC'] >= 60)) {
      if (count($regimen['Combination']) < 3) {
        $regimen = NULL;
        return;
      } elseif (count($regimen['Combination']) == 3) {
        foreach ($regimen['Combination'] as $k => $v) {
          if ((in_array(substr($regimen['Combination'][$k], 0, 3), $defs['all']['PI']) || in_array(substr($regimen['Combination'][$k], 0, 3), $defs['all']['INSTI'])) && $regimen['Mutations'][$v] <= 0) {
            $q = 1;
          }
        }
        
        if (isset($q)): hivassist_add_mutation_score_penalty ($regimen, 'ignore XTC');
        else: hivassist_add_mutation_score_penalty ($regimen);
        endif;
      } elseif (count($regimen['Combination']) >= 4) {
        hivassist_add_mutation_score_penalty ($regimen, 'ignore XTC');
      }
    } else {
      $regimen = NULL;
      return;
    }
  } else {
    hivassist_add_mutation_score_penalty ($regimen);
  }
  
/*
* Comorbidity and Interaction Rules
*
*/  
  
  // add the code to remove combinations if there is a comorbidity of 2
  if (isset($regimen['Comorbidities'])) {
    foreach ($regimen['Comorbidities'] as $k1 => $v1) {
      if (max(array_column($regimen['Comorbidities'][$k1], 'Score')) >= 2) {
        $regimen = NULL;
        return;
      }

      foreach ($regimen['Comorbidities'][$k1] as $k2 => $v2) {
        $regimen['Score'] += $regimen['Comorbidities'][$k1][$k2]['Score'];
        $regimen['Score Explanation'] .= '+ ' .$regimen['Comorbidities'][$k1][$k2]['Score']. ' (for ' .$k1. ' comorbidity with use of ' .$k2. ') <br />';
      }
    }
  }

	// add the code to remove combinations if there is a drug interaction of 2
  if (isset($regimen['Interactions'])) {
    foreach ($regimen['Interactions'] as $k1 => $v1) {
      if (max(array_column($regimen['Interactions'][$k1], 'Score')) >= 2) {
        $regimen = NULL;
        return;
      }

      foreach ($regimen['Interactions'][$k1] as $k2 => $v2) {
        $regimen['Score'] += $regimen['Interactions'][$k1][$k2]['Score'];
        $regimen['Score Explanation'] .= '+ ' .$regimen['Interactions'][$k1][$k2]['Score']. ' (for ' .$k1. ' interaction with use of ' .$k2. ') <br />';
      }
    }
  }
  
	// Remove TDF/FTC/EVGc when combined with MVC
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('MVC', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}

	// Remove TDF/FTC/EVGc when combined with EFV
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('EFV', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}

	// Remove TDF/FTC/EVGc when combined with ETR		
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('ETR', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}

	// Remove TDF/FTC/EVGc when combined with RPV		
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('RPV', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}

	// Remove TDF/FTC/EVGc when combined with ATV		
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('ATV', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}						

	// Remove TDF/FTC/EVGc when combined with ATVr
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('ATVr', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}						

	// Remove TDF/FTC/EVGc when combined with ATVc		
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('ATVc', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}		

	// Remove TDF/FTC/EVGc when combined with DRVr		
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('DRVr', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}						

	// Remove TDF/FTC/EVGc when combined with DRVc		
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('DRVc', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}										

	// Remove TDF/FTC/EVGc when combined with FPVr
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('FPVr', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}					

	// Remove TDF/FTC/EVGc when combined with LPVr		
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('LPVr', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}						

	// Remove TDF/FTC/EVGc when combined with SQVr		
	if (in_array('TDF', $regimen['Combination']) && in_array('FTC', $regimen['Combination']) && in_array('EVGc', $regimen['Combination']) && in_array('SQVr', $regimen['Combination'])) {
    $regimen = NULL;
    return;
	}
	
	if ((in_array('ETR', $regimen['Combination']) && in_array('DTG', $regimen['Combination'])) && !(in_array('ATVr', $regimen['Combination']) || in_array('DRVr', $regimen['Combination']) || in_array('LPVr', $regimen['Combination']))) {
  	$regimen = NULL;
  	return;
	}


	// add the code to remove combinations if there is an ART interaction score of 2
	if (isset($regimen['ART Interactions']) && max(array_column($regimen['ART Interactions'], 'Score')) >= 2) {
		$regimen = NULL;
    return;
	}

	// If AZT, D4T, or DDI are in the combination, give it a ranking of 6.9
	if (in_array('AZT', $regimen['Combination']) || in_array('D4T', $regimen['Combination']) || in_array('DDI', $regimen['Combination'])) {
		$regimen['Score'] = 6.9;
	}

	
	// If TAF is paired without FTC then remove it
  if (in_array('TAF', $regimen['Combination'])) { 
	  if (!in_array('FTC', $regimen['Combination'])) {
		  $regimen = NULL;
      return;
    }			
  }

	// If # of NRTIs >= 2, and
	// If TDF/TAF and FTC are not paired together, give it a ranking of 6.9
	if ($regimen['Combination Rule']['NRTI'] >= 2) {
		if (in_array('TDF', $regimen['Combination']) || in_array('TAF', $regimen['Combination'])) {
			if (!in_array('FTC', $regimen['Combination'])) {
				$regimen = NULL;
        return;
			}
		}				
	}


	if ($regimen['Combination Rule']['NRTI'] >= 2) {
  	if (!in_array('FTC', $regimen['Combination'])) {
	  	if (in_array('TDF', $regimen['Combination']) || in_array('TAF', $regimen['Combination'])) {
				$regimen = NULL;
        return;
			}
		}				
	}

	// If # of NRTIs >= 2, and
	// If 3TC and ABC are not paired together, give it a ranking of 6.9
	if ($regimen['Combination Rule']['NRTI'] >= 2) {
		if (in_array('3TC', $regimen['Combination']) || in_array('ABC', $regimen['Combination'])) {
			if (!(in_array('3TC', $regimen['Combination']) && in_array('ABC', $regimen['Combination']))) {
  			$regimen = NULL;
        return;
			}
		}
	}

	// If # of NRTIs = 2, and
	// If both 3TC and FTC are in the combination, give it a ranking of 6.9
	// Can eventually include the exception for renal insufficiency within this for loop 
	if ($regimen['Combination Rule']['NRTI'] == 2) {
		if (in_array('3TC', $regimen['Combination']) && in_array('FTC', $regimen['Combination'])) {
			$regimen = NULL;
      return;
		}
	}


	// If # of NRTIs = 2, and
	// If both TDF and ABC are in the combination, give it a ranking of 6.9
	if ($regimen['Combination Rule']['NRTI'] == 2) {
		if (in_array('TDF', $regimen['Combination']) && in_array('ABC', $regimen['Combination'])) {
			$regimen = NULL;
      return;
		}
	}
	
	if (in_array('EVGc', $regimen['Combination'])) {
    if (!in_array('FTC', $regimen['Combination']) || !(in_array('TAF', $regimen['Combination']) || in_array('TDF', $regimen['Combination']))) {
      $regimen = NULL;
      return;
    }
	}


/*
  // If FPVr or TPVr are in the combination, give it a ranking of 6.8
	if (in_array('FPVr', $regimen['Combination']) || in_array('TPVr', $regimen['Combination'])) {
		$regimen['Score'] = 6.8;
	}
*/
	
	// Add 0.1 to ranking score if ATV, ATVr, or ATVc, OR ETR is in regimen
	if (in_array('ATV', $regimen['Combination']) || in_array('ATVr', $regimen['Combination'])  || in_array('ATVc', $regimen['Combination']) || in_array('ETR', $regimen['Combination'])) {
		$regimen['Score'] = $regimen['Score'] + 0.1;
    $regimen['Score Explanation'] .= '+ 0.1 (for use of ATV or ETR) <br />';
	}

	// Add 0.2 to ranking score if LPVr OR NVP is in regimen
	if (in_array('LPVr', $regimen['Combination']) || in_array('NVP', $regimen['Combination'])) {
		$regimen['Score'] = $regimen['Score'] + 0.2;
    $regimen['Score Explanation'] .= '+ 0.2 (for use of LPV or NVP) <br />';
	}

/*
	// If IDVr, NFVr, or SQVr are in the combination, remove those combinations
	if (isset($regimen)) {
		if (in_array('IDVr', $regimen['Combination']) || in_array('NFVr', $regimen['Combination']) || in_array('SQVr', $regimen['Combination'])) {
			$regimen['Ranking'] = 6.9;
		}
	}
*/	
	
  if (in_array('DRV', $regimen['Combination']) || in_array('DRVr', $regimen['Combination']) || in_array('DRVc', $regimen['Combination'])) {
    foreach ($regimen['Mutations'] as $k => $v) {
      if (substr($k, 0, 3) == 'DRV' && $regimen['Mutations'][$k] > 0) {
        foreach ($regimen['Dosing'] as $k1 => $v1) {
          if (substr($regimen['Dosing'][$k1]['Combination'], 0, 3) == 'DRV') {
            $regimen['Dosing'][$k1]['Frequency'] = 2;
          }
        }              
      }
    }
  }
  
	if ($regimen['Combination Rule']['NRTI'] == 1 && in_array('FTC', $regimen['Combination'])) {
  	foreach ($regimens as $k => $v) {
      if (count($regimen['Combination']) == count($regimens[$k]['Combination']) && array_values(array_diff($regimen['Combination'], $regimens[$k]['Combination'])) == array('FTC') && array_values(array_diff($regimens[$k]['Combination'], $regimen['Combination'])) == array('3TC')) {
        if ($regimen['Score'] == $regimens[$k]['Score']) {
          $regimen = NULL;          
          return;
        }
      }
  	}
	}
		
  if (in_array('RAL', $regimen['Combination'])) {
    foreach ($regimens as $k => $v) {
      if (count($regimen['Combination']) == count($regimens[$k]['Combination']) && array_values(array_diff($regimen['Combination'], $regimens[$k]['Combination'])) == array('RAL') && array_values(array_diff($regimens[$k]['Combination'], $regimen['Combination'])) == array('DTG')) {
        if ($regimen['Score'] == $regimens[$k]['Score']) {
          $regimen = NULL;
          return;
        }
      }      
    }
  }

  if (in_array('FTC', $regimen['Combination']) && $regimen['Combination Rule']['NRTI'] == 1) {
    foreach ($regimens as $k => $v) {
      if (count($regimen['Combination']) == count($regimens[$k]['Combination']) && array_values(array_diff($regimen['Combination'], $regimens[$k]['Combination'])) == array('FTC') && array_values(array_diff($regimens[$k]['Combination'], $regimen['Combination'])) == array('3TC')) {
        $regimen = NULL;
        return;
      }
    }
  }

  if (isset($options['history'])) {
    foreach ($options['history'] as $k => $v) {
      if (in_array($options['history'][$k], $defs['all']['NNRTI']) && $regimen['Combination Rule']['NNRTI'] >= 1) {
        if ($options['history'][$k] == 'EFV' && in_array('EFV', $regimen['Combination'])) {
          $regimen = NULL;
          return;
        }
  
        $i = 0;
        foreach ($regimen['Combination'] as $k1 => $v1) {
          if (!in_array($regimen['Combination'][$k1], $defs['all']['NNRTI']) && $regimen['Mutations'][$v1] <= 0) {
            $i += 1;
          }
        }
          
        if ($i < 2) {
          $regimen['Score'] += 1;
          $regimen['Score Explanation'] .= '+ 1 (for NNRTI history without two fully active non-NNRTI drugs)<br />';
        }
      }
      
      if (in_array($options['history'][$k], $defs['all']['INSTI']) && $regimen['Combination Rule']['INSTI'] >= 1) {
        if (!in_array('DTG', $regimen['Combination'])) {
          $regimen['Score'] += 0.1;
          $regimen['Score Explanation'] .= '+ 0.1 (for INSTI history and non-DTG INSTI in regimen)<br />';
        }
      }
    }
  }

  if ($options['viralload'] != 'suppressed' && (!empty($mutations['list']) || !empty($options['history']))) {
    if ($regimen['Active Drugs'] < 3 & $regimen['Active Drugs'] >= 2) {
      $regimen['Score'] += 0.75;
      $regimen['Score Explanation'] .= '+ 0.75 (for not virally suppressed and >= 2 but < 3 active drugs)<br />';
    } elseif ($regimen['Active Drugs'] < 2) {
      $regimen['Score'] += 3;
      $regimen['Score Explanation'] .= '+ 3 (for not virally suppressed and < 2 active drugs)<br />';      
    } else {
      $regimen['Score'] -= 1;
      $regimen['Score Explanation'] .= '- 1 (for not virally suppressed and >= 3 active drugs)<br />';
    }    
    
    if ($regimen['Combination Rule']['INSTI'] >= 1) {
      $regimen['Score'] -= 0.25;
      $regimen['Score Explanation'] .= '- 0.25 (for not virally suppressed and INSTI-containing regimen)<br />';
    }
    
    if (in_array('DTG', $regimen['Combination'])) {
      $regimen['Score'] -= 0.25;
      $regimen['Score Explanation'] .= '- 0.25 (for not virally suppressed and DTG-containing regimen)<br />';
    }
    
    if (in_array('DRV', $regimen['Combination']) || in_array('DRVr', $regimen['Combination']) || in_array('DRVc', $regimen['Combination'])) {
      $regimen['Score'] -= 0.25;
      $regimen['Score Explanation'] .= '- 0.25 (for not virally suppressed and DRV-containing regimen)<br />';
    }
    
    if (count($regimen['Combination']) < 4 && $regimen['Combination Rule']['NRTI'] >= 2) {
      $regimen['Score'] += 0.25;
      $regimen['Score Explanation'] .= '+ 0.25 (for not virally suppressed and >= 2 NRTI-containing regimen)<br />';
    }
  }

  if ($options['pooradherence'] == 'yes' && max(array_column($regimen['Dosing'], 'Frequency')) >= 2) {
    $regimen['Score'] *= 2;
    $regimen['Score Explanation'] .= '* 2 (for bid dosing with poor adherence) <br />';
  }

  if ($options['pooradherence'] == 'yes' && $regimen['Active Drugs'] <= 2) {
    $regimen['Score'] += 1;
    $regimen['Score Explanation'] .= '+ 1 (for <=2 active drugs with poor adherence) <br />';
  }

  if ($options['pooradherence'] == 'yes' && $regimen['Active Drugs'] >= 3) {
    $regimen['Score'] -= 1;
    $regimen['Score Explanation'] .= '- 1 (for >=2 active drugs with poor adherence) <br />';
  }

  if ($options['pooradherence'] == 'yes' && $regimen['Combination Rule']['NNRTI'] >= 1 && !in_array('DTG', $regimen['Combination'])) {
    $regimen['Score'] += 1;
    $regimen['Score Explanation'] .= '+ 1 (for NNRTI with poor adherence, except if with DTG) <br />';
  }

  if ($options['pooradherence'] == 'yes' && in_array('DTG', $regimen['Combination'])) {
    $regimen['Score'] -= 0.5;
    $regimen['Score Explanation'] .= '- 0.5 (for DTG with poor adherence) <br />';
  }

  if ($options['pooradherence'] == 'yes' && (in_array('DRV', $regimen['Combination']) || in_array('DRVr', $regimen['Combination']) || in_array('DRVc', $regimen['Combination']))) {
    $regimen['Score'] -= 0.25;
    $regimen['Score Explanation'] .= '- 0.25 (for DRV with poor adherence) <br />';
  }
  
  $regimen['Score'] += 0.2 * ($regimen['Pill Burden'] * max(array_column($regimen['Dosing'], 'Frequency'))) - 0.2;
  $regimen['Score Explanation'] .= '+ ' .(0.2 * ($regimen['Pill Burden'] * max(array_column($regimen['Dosing'], 'Frequency'))) - 0.2). ' (0.2 * (' .$regimen['Pill Burden']. ' total pills * ' .max(array_column($regimen['Dosing'], 'Frequency')). ' time(s) daily) - 0.2) penalty for pill burden<br />';

  $regimen['Score Explanation'] .= '= ' .$regimen['Score']. ' (Final weighted score)';



	
	// Marivaroc specific rules here

	// If regimen rankings are > 6 then unset (but what about if someone wants AZT in their regimens... look above, change to 5.X? instead of 6.9?)
/*
	if (isset($regimen)) {
		if ($regimen['Ranking'] >= 6) {
			$regimen = NULL;
		}
	}
*/


}