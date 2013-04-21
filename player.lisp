;;; -*- Mode: Lisp; Syntax: Common-Lisp; Package: org.langband.engine -*-

#|

DESC: player.lisp - code for the character object
Copyright (c) 2000 - Stig Erik Sand�

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

|#

(in-package :org.langband.engine)

(bt:define-binary-class player ()

  (
    ;; === Need Special saving ===
  
    (name  :accessor player.name  :initform nil)
    (class :accessor player.class :initform nil)
    (race  :accessor player.race  :initform nil)
    (sex   :accessor player.sex   :initform nil)

    (base-stats    :accessor player.base-stats
		   :initform nil
		   :documentation "this is the base stats")
    (curbase-stats :accessor player.curbase-stats
		   :initform nil
		   :documentation "this is the current (possibly drained) base stats")
    (hp-table      :accessor player.hp-table
		   :initform nil
	           :documentation "Note: should be saved.")
    (equipment     :accessor player.equipment :initform nil)
    (dead-from     :accessor player.dead-from
		   :initform ""
	           :documentation "who killed the player?")

    ;; === Directly savable to binary ===

    (loc-x :accessor location-x :initform +illegal-loc-x+ :bt u16)
    (loc-y :accessor location-y :initform +illegal-loc-y+ :bt u16)

    (view-x :accessor player.view-x :initform +illegal-loc-x+ :bt u16);; wx
    (view-y :accessor player.view-y :initform +illegal-loc-y+ :bt u16);; wy
    
    (depth     :accessor player.depth     :initform 0 :bt s16)
    (max-depth :accessor player.max-depth :initform 0 :bt s16)
    
    (max-xp      :accessor player.max-xp      :initform 0 :bt u32)
    (cur-xp      :accessor player.cur-xp      :initform 0 :bt u32)
    (fraction-xp :accessor player.fraction-xp :initform 0 :bt u32) 

    (cur-hp      :accessor current-hp         :initform 0 :bt u32)
    (fraction-hp :accessor player.fraction-hp :initform 0 :bt u32)

    (cur-mana      :accessor player.cur-mana      :initform 0 :bt u32)
    (fraction-mana :accessor player.fraction-mana :initform 0 :bt u32)

    (gold        :accessor player.gold   :initform 0 :bt u32)
    (food        :accessor player.food   :initform (1- +food-full+) :bt u32)
    (energy      :accessor player.energy :initform 0 :bt u16)

    ;; === The remaining values can be calculated from the above ===
    
    (level     :accessor player.level     :initform 1)  ;; can be calculated from cur-xp
    (max-level :accessor player.max-level :initform 1)  ;; can be calculated from max-xp

    (max-hp    :accessor maximum-hp       :initform 0)   ;; can be calculated
    (max-mana  :accessor player.max-mana  :initform 0)   ;; can be calculated
    (xp-table  :accessor player.xp-table  :initform nil) ;; can be calculated
    
    (energy-use :accessor player.energy-use :initform 0)   ;; is just a temp-variable
    (leaving-p  :accessor player.leaving-p  :initform nil) ;; need to save it?
    (dead-p     :accessor player.dead-p     :initform nil) ;; need to save it?
    (speed      :accessor player.speed      :initform +speed-base+)  ;; does this change?
    

    (base-ac      :accessor player.base-ac      :initform 0)
    (ac-bonus     :accessor player.ac-bonus     :initform 0)
    (light-radius :accessor player.light-radius :initform 0)
   
    (infravision :accessor player.infravision
		 :initform 0)
    (inventory   :accessor player.inventory
		 :initform nil
		 :documentation "quick variable to equipment.backpack.content")
    (skills      :accessor player.skills
		 :initform nil)

    (modbase-stats :accessor player.modbase-stats
		   :initform nil
		   :documentation "this is the modified base stats (base + race + class + eq)")
    (active-stats :accessor player.active-stats
		  :initform nil
		  :documentation "this is the current active stat-value (curbase + race + class + eq)")
  
    ))


;; hack, remove later
(defun player.eq (pl-obj)
  (player.equipment pl-obj))

(defun (setf player.eq) (val pl-obj)
  (setf (player.equipment pl-obj) val))

(defmethod creature-alive? ((crt player))
  (not (player.dead-p crt)))

(defmethod (setf creature-alive?) (value (crt player))
  (setf (player.dead-p crt) (not value))
  (when (eq value nil)
    (setf (player.leaving-p crt) :dead)))
    

(defmethod get-xp-value ((crt player))
  (* (player.level crt) 20))


(defun get-race-name (player)
  "Returns a string with the name of the race."
  (race.name (player.race player)))

(defun get-class-name (player)
  "Returns a string with the name of the class."
  (class.name (player.class player)))

(defun get-sex-name (player)
  "Returns a string with the name of the sex."
  (cadr (assoc (player.sex player) +sexes+)))

(defmethod get-creature-ac ((crt player))
  (the fixnum (+ (the fixnum (player.base-ac crt))
		 (the fixnum (player.ac-bonus crt)))))

(defmethod get-creature-energy ((crt player))
  (the fixnum (player.energy crt)))

(defmethod (setf get-creature-energy) (val (crt player))
;;  (when (< val (player.energy crt)) (warn "Reducing energy from ~a to ~a" (player.energy crt) val))
  (setf (player.energy crt) val))

(defmethod get-creature-speed ((crt player))
  (the fixnum (player.speed crt)))

(defmethod (setf get-creature-speed) (val (crt player))
  (setf (player.speed crt) val))

(defun %make-level-array (var-obj)
  (check-type var-obj variant)
  (let ((max-char-level (variant.max-charlevel var-obj)))
    (make-array max-char-level :initial-element 0)))



(defmethod produce-player-object ((variant variant))
  "Creates and returns a PLAYER object."
  (let ((t-p (make-instance 'player)))
  
    (setf (player.base-stats t-p)    (make-stat-array)
	  (player.curbase-stats t-p) (make-stat-array)
	  (player.modbase-stats t-p) (make-stat-array)
	  (player.active-stats t-p)  (make-stat-array))

    (assert (let ((bstat-table (player.base-stats t-p))
		  (cstat-table (player.curbase-stats t-p))
		  (mstat-table (player.modbase-stats t-p))
		  (astat-table (player.active-stats t-p)))
	      
	      (and (not (eq bstat-table cstat-table))
		   (not (eq bstat-table mstat-table))
		   (not (eq bstat-table astat-table))
		   (not (eq cstat-table mstat-table))
		   (not (eq cstat-table astat-table))
		   (not (eq mstat-table astat-table)))))
    
    (setf (player.skills t-p) (produce-skills-object variant))
    (setf (player.eq t-p) (make-equipment-slots))
    
    (setf (player.hp-table t-p) (%make-level-array variant)
	  (player.xp-table t-p) (%make-level-array variant)
	  )

    (flet ((make-and-assign-backpack! (id)
	     (let ((back-obj (create-aobj-from-id id))
		   (eq-slots (player.eq t-p)))
	       ;;(warn "eq-slots ~a" eq-slots)
	       (item-table-add! eq-slots back-obj 'eq.backpack)
	       (setf (player.inventory t-p) back-obj))))
      
      (let ((backpack-val (game-parameter-value :initial-backpack)))
	(case backpack-val
	  (:backpack (make-and-assign-backpack! :backpack))
	  (otherwise
	   (warn "No initial known backpack-setting, assuming :backpack")
	   (make-and-assign-backpack! :backpack)))))

    ;; hack
    ;;    (setf (player.light-radius t-p) 3)
    
    t-p))


(defun get-stat-bonus (player stat-num)
  "Returns the stat-bonus from race, class and equipment for given stat"
  
  (let ((race-mod (race.stat-changes (player.race player)))
	(class-mod (class.stat-changes (player.class player))))
    
    ;; iterate through equipment
    (the fixnum (+ (the fixnum (svref race-mod stat-num))
		   (the fixnum (svref class-mod stat-num))))))

(defun add-stat-bonus (base amount)
  "Returns a numeric value with base incremented with amount"
  (declare (type fixnum base amount))
  (let ((retval base))
    (declare (type fixnum retval))
    (if (< amount 0)
	(dotimes (i (abs amount))
	  (cond ((>= retval (+ 18 10))
		 (decf retval 10))
		((> retval 18) ;; hackish
		 (setq retval 18))
		((> retval 3) ;; minimum
		 (decf retval 1))
		(t
		 ;; too low to care
		 )))
	;; positive amount
	(dotimes (i amount)
	  (if (< retval 18)
	      (incf retval)
	      (incf retval 10))))
    retval))
		

(defun calculate-stat! (player num)
  "modifies appropriate arrays.."

  (assert (and (>= num 0)
	       (< num +stat-length+)))
  
  (let ((bstat-table (player.base-stats player))
	(cstat-table (player.curbase-stats player))
	(mstat-table (player.modbase-stats player))
	(astat-table (player.active-stats player)))

    (assert (and (not (eq bstat-table cstat-table))
		 (not (eq bstat-table mstat-table))
		 (not (eq bstat-table astat-table))
		 (not (eq cstat-table mstat-table))
		 (not (eq cstat-table astat-table))
		 (not (eq mstat-table astat-table))))
		 
    ;; two of these are required.. bstat and cstat

  
    (let ((base-stat (svref bstat-table num))
	  (cur-stat (svref cstat-table num))
	  (bonus (get-stat-bonus player num)))

      (setf (svref mstat-table num) (add-stat-bonus base-stat bonus))
      (setf (svref astat-table num) (add-stat-bonus cur-stat bonus))
      )
    
    (values)))

;;(trace add-stat-bonus get-stat-bonus)

(defmethod update-player! ((variant variant) (player player))
  "modifies player object appropriately"

  ;; we start the show by reseting variables

  ;; reset some key variables
  (setf (player.base-ac player) 0
	(player.ac-bonus player) 0
	(player.light-radius player) 0
	(player.speed player) +speed-base+)
  
  
  (let ((race (player.race player)))

    ;; if cur and base are missing, that should be fixed elsewhere
    
    (unless (arrayp (player.modbase-stats player))
      (setf (player.modbase-stats player) (make-stat-array)))
    (unless (arrayp (player.active-stats player))
      (setf (player.active-stats player) (make-stat-array)))

    ;; check that they're all there
    (assert (and (arrayp (player.base-stats player))
		 (arrayp (player.curbase-stats player))
		 (arrayp (player.active-stats player))
		 (arrayp (player.modbase-stats player))))


    (dotimes (i +stat-length+)
;;      (warn ">B fore ~s" (svref (player.base-stats player) i))
      (calculate-stat! player i)
;;      (warn ">B after ~s" (svref (player.base-stats player) i))
      )
    


    ;; hackish, change later
    (let ((race-ab (race.abilities race)))
      (dolist (i race-ab)
;;	(Warn "checking ~a" i)
	(when (consp i)
	  (case (car i)
	    (<infravision> (setf (player.infravision player) (cadr i)))
	    (<resist> ;; handle later
	     )
	    (<sustain> ;; handle later
	     )
	    (otherwise
	     #+cmu ;; FIX
	     (warn "Unhandled racial ability ~a" (car i))
	     )))))


     
    ;; let us skim through items and update variables
    (let ((slots (player.eq player)))
      (unless slots
	(error "Can't find equipment-slots for player, bad."))
      (flet ((item-iterator (table key obj)
	       (declare (ignore table key))
	       (when obj
		 (let* ((kind (aobj.kind obj))
			(gval (object.game-values kind)))
		   (when gval
		     (when (> (gval.light-radius gval) (player.light-radius player))
		       (setf (player.light-radius player) (gval.light-radius gval)))
		     (incf (player.base-ac player) (gval.base-ac gval))
		     (incf (player.ac-bonus player) (gval.ac-bonus gval)))))))
	(declare (dynamic-extent #'item-iterator))
	(item-table-iterate! slots #'item-iterator)))


    ;; do this more intelligently later..
    (let ((tbl (player.xp-table player)))
      (when (or (eq nil tbl) (and (arrayp tbl)
				  (= 0 (aref tbl 1)))) ;; hack
	(update-xp-table! variant player)))

    (let ((xp-table (player.xp-table player)))
      (setf (player.level player) (find-level-for-xp (player.cur-xp player)
						     xp-table)
	    (player.max-level player) (find-level-for-xp (player.max-xp player)
							 xp-table)))

    (update-max-hp! player)
    (update-skills! player (player.skills player))
    
    (bit-flag-add! *redraw* +print-basic+)

    ;; when leavin we should be ok
    (assert (ok-object? player))
    player))


(defun gain-level! (player)
  "lets the player gain a level.. woah!  must be updated later"

  (let* ((the-level (player.level player))
	 (hp-table (player.hp-table player))
	 (next-hp (aref hp-table the-level)))

    ;; we have been to this level earlier..
    (when (or (eq next-hp nil) (< next-hp 1))
      (let* ((the-class (player.class player))
	     (the-race (player.race player))
	     (hit-dice (the fixnum (+ (the fixnum (class.hit-dice the-class))
				      (the fixnum (race.hit-dice the-race))))))
	(setq next-hp (randint hit-dice))
	(setf (aref hp-table the-level) next-hp)))

    (incf (maximum-hp player) next-hp)
    (incf (player.level player))

    (when (< (player.max-level player) (player.level player))
      (setf (player.max-level player)  (player.level player)))

    (with-foreign-str (s)
      (lb-format s "You attain level ~d and ~d new hitpoints. " (player.level player) next-hp)
      (c-print-message! s))
    
    ))

(defun find-level-for-xp (xp xp-table)
  "Returns level for given xp according to given xp-table."
  (loop for x across xp-table
	for i of-type fixnum from 1
	do
	(when (> x xp)
;;	  (warn "Returning lvl ~s for xp ~s" (1- i) xp)
	  (return-from find-level-for-xp (1- i))))
  1) ;; fix me later

;;(trace find-level-for-xp)

(defmethod alter-xp! ((player player) amount)
  "Alters the xp for the player with the given amount."

  (assert (numberp amount))
  
  (when (minusp amount)
    (warn "Not implemented reduction in XP yet.")
    (return-from alter-xp! nil))

  (when (= amount 0)
    (return-from alter-xp! nil))
  
  (incf (player.cur-xp player) amount)
  (incf (player.max-xp player) amount)

  (loop
   (let* ((cur-level (player.level player))
	  (next-limit (aref (player.xp-table player) cur-level))
	  (cur-xp (player.cur-xp player)))

;;     (warn "comparing ~s and ~s at lvl ~s -> ~a" cur-xp next-limit cur-level (> cur-xp next-limit))
     (if (>= cur-xp next-limit)
	 (gain-level! player)
	 (return-from alter-xp! nil)))
  
   ))


(defmethod get-weapon ((crt player))
  (let ((the-eq (player.eq crt)))
    (check-type the-eq item-table)
    
    (item-table-find the-eq 'eq.weapon)))

(defmethod get-missile-weapon ((crt player))
  (let ((the-eq (player.eq crt)))
    (check-type the-eq item-table)
    
    (item-table-find the-eq 'eq.bow)))


(defun reset-skills! (variant skills-obj reset-val)
  "Sets all skills to RESET-VAL."
  (dolist (i (variant.skill-translations variant))
    (setf (slot-value skills-obj (cdr i)) reset-val)))


(defun update-skills! (player skills-obj)
  "Recalculates and cleans up as needed."

  (flet ((add-to-skill! (which the-skills-obj player-lvl source)
	   (declare (type fixnum player-lvl))
	   (when source
	     (let ((obj (slot-value source which)))
	       (if (not obj)
		   (warn "Skill-slot ~s does have NIL value, please fix." which)
		   (incf (slot-value the-skills-obj which)
			 (cond ((eq obj nil) 0)
			       ((numberp obj) obj)
			       ((skill-p obj)
				(the fixnum (+ (the fixnum (skill.base obj))
					       (int-/ (* player-lvl (the fixnum (skill.lvl-gain obj)))
						      10))))
			       (t
				(error "Unknown skill-obj ~a" obj)))))))))
    (declare (dynamic-extent #'add-to-skill!))
    (let* ((var-obj *variant*)
	   (race (player.race player))
	   (the-class (player.class player))
	   (racial-skills (race.skills race))
	   (class-skills (class.skills the-class))
	   (player-lvl (player.level player))
	   (skill-list (variant.skill-translations var-obj)))
      
      ;; reset to value 0 first
      (reset-skills! var-obj skills-obj 0)
      
      (dolist (i skill-list)
	
	(add-to-skill! (cdr i)
		       skills-obj
		       player-lvl
		       racial-skills)
	(add-to-skill! (cdr i)
		       skills-obj
		       player-lvl
		       class-skills))
      
      ;;    (describe skills-obj)
      
      t)))

(defmethod update-xp-table! ((variant variant) (player player))
  "Updates the xp-table on the player, and returns updated player."
  
  (let* ((base-xp-table (variant.xp-table variant))
	 (max-char-level (variant.max-charlevel variant))
	 (the-race (player.race player))
	 (the-class (player.class player))
	 (xp-extra (+ 100
		      (race.xp-extra the-race)
		      (class.xp-extra the-class))))

    (unless (arrayp (player.xp-table player))
      (setf (player.xp-table player) (%make-level-array variant)))

    (let ((xp-table (player.xp-table player)))
      
      (setf (aref xp-table 0) 0)
      (loop for i of-type u-fixnum from 1 to (1- max-char-level)
	    do
	    (setf (aref xp-table i) (int-/ (* (aref base-xp-table (1- i)) xp-extra)
					   100))))
    player))

(defun update-max-hp! (player)
  "Updates the maximum number of hitpoints.  Returns an updated player."

  (let ((lvl (player.level player))
	(hp-table (player.hp-table player)))
    
    (setf (maximum-hp player)
	  (loop for i from 0 to (1- lvl)
		summing (aref hp-table i))))
  player)
	   
(defmethod heal-creature! ((pl player) amount)
  "Heals the player and adds notify where needed."

  (let ((max-hp (maximum-hp pl)))
  
    (when (< (current-hp pl) max-hp)
      
      (incf (current-hp pl) amount)
      
      (when (< max-hp (current-hp pl)) ;; no more than max..
	(setf (current-hp pl) max-hp
	      (player.fraction-hp pl) 0))
      
      (bit-flag-add! *redraw* +print-hp+)
      
      ;; message
      (cond ((< amount 5)
	     (c-print-message! "You feel a little better."))
	    ((< amount 15)
	     (c-print-message! "You feel better."))
	    ((< amount 35)
	     (c-print-message! "You feel much better."))
	    (t
	     (c-print-message! "You feel very good.")))
      
      t))) ;; it returns nil if when doesn't make sense

(defmethod set-creature-state! ((crt player) state value)

  ;; :fear
  ;; :hero
  ;; :blindness
  ;; :heal-cut, <light> (c-10), <serious> ((c/2)-50)
  ;; :confusion
  ;; :poison nil + :slow, num
  ;; :confusion
  ;; :stun
  ;; :cut
  ;; :hallucination
  ;; :berserk
  
  (case state
    (:fear (warn "Setting fear of player to ~s" value))
    (otherwise (warn "Unknown state for player: ~s" state)))
  
  nil)

(defmethod possible-identify! ((player player) (obj active-object))
  (learn-about-object! player obj :tried)
  ;; fix later
  (learn-about-object! player obj :aware)
  (learn-about-object! player obj :known)
  ;; add xp?
  )

;; Deprecated.. 
(defmethod possible-identify! ((player player) (obj object-kind))
  (warn "Deprecated identify-method called on ~s" obj)
  (learn-about-object! player obj :tried)
  ;; fix later
  (learn-about-object! player obj :aware)
;;  (learn-about-object! pl obj :known)
  ;; add xp?
  )


(defun update-player-stat! (pl stat action)
  "Action can be <restore> or a positive or negative integer."

;;  (declare (ignore pl stat action))

  ;; currently does restore
  (let ((bs (player.base-stats pl))
	(cbs (player.base-stats pl))
	(num (etypecase stat
	       (number stat)
	       (symbol (get-stat-num-from-sym stat)))))

    (ecase action
      (<restore>
       (let ((bval (aref bs num))
	     (cbval (aref cbs num)))
	 (when (< cbval bval)
	   (with-foreign-str (s)
	     (lb-format s "You feel less ~s" (get-stat-name-from-sym stat))
	     (c-print-message! s))
	   (setf (aref cbs num) (aref bs num))
	   t)))
      
      (<increase>
       (warn "increase stat not implemented.")
       t)
      (<reduce>
       (warn "reduce stat not implemented.")
       t))

    ))


(defun alter-food! (pl new-food-amount)
  ;; lots of minor pooh
  (setf (player.food pl) new-food-amount))

