package com.uap.cse316.smartclassroom.ui.main

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import com.uap.cse316.smartclassroom.R
import com.uap.cse316.smartclassroom.databinding.ActivityMainBinding
import com.uap.cse316.smartclassroom.ui.alerts.AlertsFragment
import com.uap.cse316.smartclassroom.ui.dashboard.DashboardFragment
import com.uap.cse316.smartclassroom.ui.history.HistoryFragment
import com.uap.cse316.smartclassroom.ui.login.LoginActivity

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private val dashboardFragment = DashboardFragment()
    private val alertsFragment = AlertsFragment()
    private val historyFragment = HistoryFragment()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val userRole = intent.getStringExtra("USER_ROLE") ?: "Faculty"
        binding.topAppBar.subtitle = "UAP CSE 316 · $userRole"

        binding.topAppBar.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {
                R.id.action_logout -> {
                    startActivity(Intent(this, LoginActivity::class.java))
                    finish()
                    true
                }
                else -> false
            }
        }

        // Set initial fragment
        loadFragment(dashboardFragment)

        // Bottom Navigation Handler
        binding.bottomNavigation.setOnItemSelectedListener { item ->
            when (item.itemId) {
                R.id.nav_dashboard -> {
                    loadFragment(dashboardFragment)
                    true
                }
                R.id.nav_alerts -> {
                    loadFragment(alertsFragment)
                    true
                }
                R.id.nav_history -> {
                    loadFragment(historyFragment)
                    true
                }
                else -> false
            }
        }
    }

    private fun loadFragment(fragment: Fragment) {
        supportFragmentManager.beginTransaction()
            .replace(R.id.fragmentContainer, fragment)
            .commit()
    }
}
