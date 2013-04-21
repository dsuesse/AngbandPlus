;;; -*- Mode: Lisp; Syntax: Common-Lisp; Package: org.langband.engine -*-

#||

DESC: monster.lisp - monster-code
Copyright (c) 2000-2001 - Stig Erik Sand�

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

----

ADD_DESC: The code which deals with critters you can meet in the dungeon.

||#

(in-package :org.langband.engine)
  
(defclass monster-kind ()
  ((id        :initarg :id   :accessor monster.id        :initform nil)
   (name      :initarg :name :accessor monster.name      :initform nil)
   (desc      :accessor monster.desc      :initform nil)
   (symbol    :accessor monster.symbol    :initform nil)
   (colour    :accessor monster.colour    :initform nil)
   (alignment :accessor monster.alignment :initform nil)
   (type      :accessor monster.type      :initform nil)
   (level     :accessor monster.level     :initform nil)
   (rarity    :accessor monster.rarity    :initform nil)
   (hitpoints :accessor monster.hitpoints :initform nil)
   (armour    :accessor monster.armour    :initform nil)
   (speed     :accessor monster.speed     :initform 0)
   (xp        :accessor monster.xp        :initform 0)
   (sex       :accessor monster.sex       :initform nil)

   (abilities  :accessor monster.abilities  :initform nil)
   ;;   (resists :accessor monster.resists :initform nil)
   (immunities :accessor monster.immunities :initform nil)

   (alertness  :accessor monster.alertness  :initform nil)
   (vision     :accessor monster.vision     :initform nil)
   (attacks    :accessor monster.attacks    :initform nil)
   (treasures  :accessor monster.treasures  :initform nil)
   
   (vulnerabilities   :accessor monster.vulnerabilities :initform nil)
   (special-abilities :accessor monster.sp-abilities    :initform nil)
   )) 

(defmethod has-ability? ((mon monster-kind) ability)
  (dolist (i (monster.abilities mon))
    (cond ((and (symbolp i) (eq ability i))
	   (return-from has-ability? i))
	  ((and (consp i) (eq (car i) ability))
	   (return-from has-ability? i))))
  nil)
	  
(defmethod has-ability? ((mon active-monster) ability)
  (has-ability? (amon.kind mon) ability))

(defmethod produce-monster-kind ((variant variant) id name &key the-kind)
  (declare (ignore the-kind))
  (assert (stringp id))
  (assert (stringp name))
  (make-instance 'monster-kind :id id :name name))

(defmethod produce-active-monster ((variant variant) mon-type)

  (assert (not (eq mon-type nil)))

  
  (let* ((the-kind (cond ((symbolp mon-type)
			  (get-monster-kind variant (symbol-name mon-type)))
			 ((stringp mon-type)
			  (get-monster-kind variant mon-type))
			 ((typep mon-type 'monster-kind)
			  mon-type)
			 (t
			  (error "Mon-type argument to produce-active-monster is not {symbol,string,mkind}, but is ~s"
				 mon-type))))
			  
	 (amon (make-instance 'active-monster :kind the-kind))
	 (num-hitdice (car (monster.hitpoints the-kind)))
	 (hitdice (cdr (monster.hitpoints the-kind))))
    
    (if (has-ability? the-kind '<max-hitpoints>)
	(setf (current-hp amon) (* num-hitdice hitdice))
	(setf (current-hp amon) (roll-dice num-hitdice hitdice)))

    (setf (maximum-hp amon) (current-hp amon))
    (setf (get-creature-speed amon) (monster.speed the-kind))
;;    (warn "Monster ~a got ~a hp from ~a dice" (get-creature-name amon)
;;	  (current-hp amon) (monster.hitpoints kind))
    ;; blah
    amon))



(defmethod get-xp-value ((creature active-monster))
  (monster.xp (amon.kind creature)))

(defmethod get-creature-name ((creature active-monster))
  (monster.name (amon.kind creature)))

(defmethod get-creature-ac ((creature active-monster))
  (monster.armour (amon.kind creature)))


(defmethod monster.name ((creature active-monster)) ;; remove eventually
  (monster.name (amon.kind creature)))

(defmethod get-creature-name ((creature player))
  (player.name creature))

(defmethod alter-xp! ((mon active-monster) amount)
  (declare (ignore amount))
  nil)

(defmethod monster.attacks ((mon active-monster))
  (monster.attacks (amon.kind mon)))

(defmethod get-mkind-table ((var-obj variant) (level level))
  
  (let* ((o-table (get-mtype-table var-obj level))
	 (table (gobj-table.obj-table o-table)))
    table))

(defmethod get-mkind-alloc-table ((var-obj variant) (level level))
  
  (let* ((o-table (get-mtype-table var-obj level))
	 (table (gobj-table.alloc-table o-table)))
    table))
 

(defmethod get-monster-kind ((variant variant) id)
  "Returns monster-kind or nil.  I think this one is quite limited."
  (assert (or (stringp id) (symbolp id)))
  (let ((table (get-mkind-table variant *level*))
	(key (if (symbolp id) (symbol-name id) id)))
;;    (warn "htbl has test ~a" (hash-table-test table))
    (gethash key table)))


(defmethod convert-obj (attacks (to (eql :attk-list)) &key)
  (mapcar #'(lambda (x)
	      (list (attack.kind x) :type (attack.dmg-type x) :damage (attack.damage x)))
	  attacks))

(defmethod convert-obj (attk-list (to (eql :attacks)) &key)
  "Converts attacks in list-form to a list of attack instances."
  (let ((attacks '()))
    (dolist (i attk-list)
      (cond ((consp i)
	     (assert (symbolp (car i)))
	     (let ((attk (make-instance 'attack :kind (car i))))
	       (do ((x (cdr i) (cddr x)))
		   ((null x))
		 (ecase (car x)
		   (:type (setf (attack.dmg-type attk) (cadr x)))
		   (:damage (setf (attack.damage attk) (cadr x)))))

	       (push attk attacks)))
	    (t
	     (warn "Unknown attack-info ~s" i))))
    (nreverse attacks)))


(defmethod get-monster-list ((var-obj variant))
  "returns a fresh list.  Remove me!"
  (let ((table (get-mkind-table var-obj *level*)))
    (stable-sort (loop for v being each hash-value of table
		       collecting v)
		 #'string<
		 :key #'monster.id)))

(defun get-all-monsters (&key (var-obj *variant*))
  "Returns a fresh list."
  (let* ((mon-info (variant.monsters var-obj))
	 (mon-tables (loop for x being each hash-value of mon-info
			   collecting (gobj-table.obj-table x)))
	 (total-list '()))
    (dolist (i mon-tables)
      (loop for v being each hash-value of i
	    do (push v total-list)))
    (stable-sort total-list #'string< :key #'monster.id)))

(defun define-monster-kind (id name &key desc symbol colour
			    alignment type level
			    rarity hitpoints armour
			    speed xp abilities
			    immunities alertness
			    vulnerabilities
			    vision attacks special-abilities
			    treasures sex)
  "Defines a critter you might bump into when you least expect it. It uses
the *VARIANT* object so it has to be properly initialised."
  
;;  (lang-warn "Creating monster ~a [~a]" name id)

  (assert (or (stringp id) (symbolp id)))
  (assert (stringp name))
    
  (let* ((var-obj *variant*)
	 (m-obj (produce-monster-kind var-obj
				      (if (symbolp id)
					  (string-downcase (symbol-name id))
					  id)
				      name)))
    
    (if (stringp desc)
	(setf (monster.desc m-obj) desc)
	(warn "No description for monster ~a found" id))

    (when symbol
      (setf (monster.symbol m-obj) symbol))
    (when colour
      (setf (monster.colour m-obj) (etypecase colour
				     (number (charify-number colour))
				     (character (convert-obj colour :colour-code))
				     )))
    (when xp
      (setf (monster.xp m-obj) xp))
    (when speed
      (setf (monster.speed m-obj) speed))
    (when hitpoints
      (setf (monster.hitpoints m-obj) hitpoints))
    (when armour
      (setf (monster.armour m-obj) armour))
    (when type
      (setf (monster.type m-obj) type))
    (when alignment
      (setf (monster.alignment m-obj) alignment))
    (when sex
      (setf (monster.sex m-obj) sex))
    
    (if (and level (typep level '(integer 0 *))) 
	(setf (monster.level m-obj) level)
	(progn
	  (lang-warn "Given illegal level-value ~s for monster ~s" level name)
	  (setf (monster.level m-obj) 1))) ;; hack

    (if (and rarity (typep rarity '(integer 0 *))) 
	(setf (monster.rarity m-obj) rarity)
	(progn
	  (lang-warn "Given illegal rarity-value ~s for monster ~s" rarity name)
	  (setf (monster.rarity m-obj) 1))) ;; hack

    (when abilities
      (setf (monster.abilities m-obj) abilities))
    
    (when alertness
      (setf (monster.alertness m-obj) alertness))
    (when immunities
      (setf (monster.immunities m-obj) immunities))
    (when vulnerabilities
      (setf (monster.vulnerabilities m-obj) vulnerabilities))
    (when vision
      (setf (monster.vision m-obj) vision))
    (when attacks
      (cond ((consp attacks)
	     (setf (monster.attacks m-obj) (convert-obj attacks :attacks)))
	    (t
	     (lang-warn "Unknown form of attacks-argument ~s for monster ~s" attacks name))))

    (when treasures
      (setf (monster.treasures m-obj) treasures))
    (when special-abilities
      (setf (monster.sp-abilities m-obj) special-abilities))
    

    ;; applies the filters registered for newly read monsters
    (apply-filters-on-obj :monsters var-obj m-obj)
    ;;(add-new-mkind! m-obj id)
    
    m-obj))

(defmethod get-loadable-form ((object monster-kind) &key)
  (let ((the-form '()))
    (flet ((possibly-add (initarg val &optional (def-val nil))
	     (unless (equal val def-val)
	       (setf the-form (nconc the-form (list initarg (loadable-val val)))))))
      
      (setf the-form (list 'define-monster-kind 
			   (monster.id object)
			   (monster.name object)))
      
      (possibly-add :desc (monster.desc object))
      (possibly-add :symbol (monster.symbol object))
      (possibly-add :colour (convert-obj (monster.colour object) :letter))
      (possibly-add :alignment (monster.alignment object))
      (possibly-add :type (monster.type object))
      (possibly-add :level (monster.level object))
      (possibly-add :rarity (monster.rarity object))
      (possibly-add :hitpoints (monster.hitpoints object))
      (possibly-add :armour (monster.armour object))
      (possibly-add :speed (monster.speed object) 0)
      (possibly-add :xp (monster.xp object) 0)
      (possibly-add :abilities (monster.abilities object))
      (possibly-add :immunities (monster.immunities object))
      (possibly-add :vulnerabilities (monster.vulnerabilities object))
      (possibly-add :alertness (monster.alertness object))
      (possibly-add :vision (monster.vision object))
      (possibly-add :attacks (convert-obj (monster.attacks object) :attk-list))
      (possibly-add :treasures (monster.treasures object))
      (possibly-add :sex (monster.sex object))
      (possibly-add :special-abilities (monster.sp-abilities object))

      the-form)))

      
(defun dump-monsters (out-file &key (monster-list nil) (var-obj *variant*))
  (let ((mon-list (if monster-list
		       monster-list
		       (get-monster-list var-obj)))
	(*print-case* :downcase)
	(*print-right-margin* 120))
    
    (with-open-file (ffile (pathname out-file)
			   :direction :output
			   :if-exists :supersede
			   :if-does-not-exist :create)
      (pprint '(in-package :langband) ffile)
      (terpri ffile)
      (dolist (x mon-list)
	(print (get-loadable-form x) ffile)
	(terpri ffile))
      (terpri ffile))))


(defmethod print-object ((inst monster-kind) stream)
  (print-unreadable-object
   (inst stream :identity t)
   (format stream "~:(~S~) [~S ~S]" (class-name (class-of inst)) 
	   (monster.id inst)
	   (monster.name inst)))
  inst)


(defmethod print-object ((inst active-monster) stream)
  (print-unreadable-object
   (inst stream :identity t)
   (format stream "~:(~a~) [~S, (~s,~s)]" (class-name (class-of inst)) 
	   (amon.kind inst) (location-x inst) (location-y inst))
  inst))

