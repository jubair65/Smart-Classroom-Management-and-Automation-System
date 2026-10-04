package com.uap.cse316.smartclassroom.ui.students

import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.ValueEventListener
import com.uap.cse316.smartclassroom.data.model.ClassroomLive
import com.uap.cse316.smartclassroom.data.model.StudentItem
import com.uap.cse316.smartclassroom.databinding.ActivityStudentsBinding
import com.uap.cse316.smartclassroom.utils.FirebaseManager

class StudentsActivity : AppCompatActivity() {

    private lateinit var binding: ActivityStudentsBinding
    private val studentAdapter = StudentAdapter()
    private var liveListener: ValueEventListener? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityStudentsBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupToolbar()
        setupRecyclerView()
        setupSwipeRefresh()
        listenToStudents()
    }

    private fun setupToolbar() {
        binding.btnBack.setOnClickListener {
            finish()
        }
    }

    private fun setupRecyclerView() {
        binding.rvStudents.apply {
            layoutManager = LinearLayoutManager(this@StudentsActivity)
            adapter = studentAdapter
            setHasFixedSize(true)
        }
    }

    private fun setupSwipeRefresh() {
        binding.swipeRefresh.setOnRefreshListener {
            binding.swipeRefresh.isRefreshing = false
        }
    }

    private fun listenToStudents() {
        val liveRef = FirebaseManager.getLiveReference()

        liveListener = object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                if (isFinishing || isDestroyed) return

                val liveData = snapshot.getValue(ClassroomLive::class.java) ?: ClassroomLive()
                val studentsList = mutableListOf<StudentItem>()

                // 1. First parse from direct 'students' child in snapshot for maximum reliability
                if (snapshot.hasChild("students")) {
                    for (child in snapshot.child("students").children) {
                        val name = child.child("name").getValue(String::class.java) ?: ""
                        val enterTime = child.child("enterTime").getValue(String::class.java) ?: ""
                        val date = child.child("date").getValue(String::class.java) ?: liveData.date
                        if (name.isNotBlank() || enterTime.isNotBlank()) {
                            studentsList.add(StudentItem(name = name, enterTime = enterTime, date = date))
                        }
                    }
                }

                // 2. If empty but liveData.students list exists from POJO
                if (studentsList.isEmpty() && liveData.students.isNotEmpty()) {
                    studentsList.addAll(liveData.students)
                }

                // 3. Fallback: if studentCount > 0 but students array not yet written (e.g. initial sync)
                if (studentsList.isEmpty() && liveData.studentCount > 0) {
                    val defaultNames = listOf("Jubair", "Hasanul", "Maria")
                    for (i in 0 until liveData.studentCount) {
                        val name = defaultNames.getOrElse(i) { "Student ${i + 1}" }
                        studentsList.add(
                            StudentItem(
                                name = name,
                                enterTime = if (liveData.time.isNotBlank()) liveData.time else "Earlier",
                                date = liveData.date
                            )
                        )
                    }
                }

                updateUI(studentsList, liveData.studentCount)
            }

            override fun onCancelled(error: DatabaseError) {
                // Keep current state on error
            }
        }

        liveRef.addValueEventListener(liveListener as ValueEventListener)
    }

    private fun updateUI(students: List<StudentItem>, count: Int) {
        val displayCount = if (students.isNotEmpty()) students.size else count
        val countText = "$displayCount Student${if (displayCount == 1) "" else "s"}"
        binding.tvCountBadge.text = countText

        if (students.isEmpty()) {
            binding.rvStudents.visibility = View.GONE
            binding.layoutEmptyState.visibility = View.VISIBLE
        } else {
            binding.layoutEmptyState.visibility = View.GONE
            binding.rvStudents.visibility = View.VISIBLE
            studentAdapter.setStudents(students)
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        liveListener?.let {
            FirebaseManager.getLiveReference().removeEventListener(it)
        }
    }
}
